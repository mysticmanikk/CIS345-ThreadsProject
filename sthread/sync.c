/*
 * NAME, etc.
 *
 * sync.c
 *
 * Synchronization routines for SThread
 */

#define _REENTRANT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "sthread.h"

/*
 * Atomic operations for x86 architecture.
 */
static inline int test_and_set_bit(volatile unsigned long *addr)
{
	int oldval;
	__asm__ __volatile__("xchgl %0, %1"
			: "=r"(oldval), "+m"(*(addr))	/* output */
			: "0"(1)						/* input */
			: "memory"	/* clobbered: changing contents of memory */
			);
	return oldval;
}
static inline void clear_bit(volatile unsigned long *addr)
{
	unsigned long oldval;
	__asm__ __volatile__("xchgl %0, %1"
			: "=r"(oldval), "+m"(*(addr))	/* output */
			: "0"(0)						/* input */
			: "memory"	/* clobbered: changing contents of memory */
			);
}

/*
 * rwlock routines
 */

// Initialize the read-write lock
int sthread_rwlock_init(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    rwlock->active_readers = 0;
    rwlock->active_writers = 0;
    rwlock->waiting_writers_count = 0;
    rwlock->waiting_readers = NULL;
    rwlock->waiting_writers = NULL;
    rwlock->lock = 0;

    return 0;
}

// Destroy the read-write lock
int sthread_rwlock_destroy(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    // Clean up the queues
    while (rwlock->waiting_readers) {
        sthread_queue_t *temp = rwlock->waiting_readers;
        rwlock->waiting_readers = rwlock->waiting_readers->next;
        free(temp);
    }

    while (rwlock->waiting_writers) {
        sthread_queue_t *temp = rwlock->waiting_writers;
        rwlock->waiting_writers = rwlock->waiting_writers->next;
        free(temp);
    }

    return 0;
}

// Acquire the read lock
int sthread_read_lock(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    while (1) {
        // Lock acquired
        if (test_and_set_bit(&rwlock->lock) == 0) {
            // Ensure no writers are active or waiting
            if (rwlock->active_writers == 0 && rwlock->waiting_writers_count == 0) {
                rwlock->active_readers++;
                clear_bit(&rwlock->lock);
                return 0;
            }
            clear_bit(&rwlock->lock);  // Release the lock
        }

        // Queue the reader and suspend
        sthread_queue_t *new_reader = malloc(sizeof(sthread_queue_t));
        new_reader->thread = sthread_self();
        new_reader->next = NULL;

        if (!rwlock->waiting_readers) {
            rwlock->waiting_readers = new_reader;
        } else {
            sthread_queue_t *temp = rwlock->waiting_readers;
            while (temp->next) temp = temp->next;
            temp->next = new_reader;
        }

        sthread_suspend(); // Suspend the thread
    }
}

// Try to acquire the read lock (non-blocking)
int sthread_read_try_lock(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    // Try to acquire the lock without blocking
    if (test_and_set_bit(&rwlock->lock) == 0) {
        if (rwlock->active_writers == 0 && rwlock->waiting_writers_count == 0) {
            rwlock->active_readers++;
            clear_bit(&rwlock->lock);
            return 0;
        }
        clear_bit(&rwlock->lock);  // Release the lock
    }

    return EBUSY;  // Return immediately if the lock can't be acquired
}

// Release the read lock
int sthread_read_unlock(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    test_and_set_bit(&rwlock->lock); // Acquire the lock
    rwlock->active_readers--;

    // If no more active readers, wake a waiting writer if any
    if (rwlock->active_readers == 0 && rwlock->waiting_writers) {
        sthread_queue_t *writer = rwlock->waiting_writers;
        rwlock->waiting_writers = rwlock->waiting_writers->next;
        rwlock->waiting_writers_count--;
        sthread_wake(writer->thread);
        free(writer);
    }

    clear_bit(&rwlock->lock); // Release the lock
    return 0;
}

// Acquire the write lock
int sthread_write_lock(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    while (1) {
        if (test_and_set_bit(&rwlock->lock) == 0) {
            if (rwlock->active_readers == 0 && rwlock->active_writers == 0) {
                rwlock->active_writers++;
                clear_bit(&rwlock->lock);
                return 0;
            }
            clear_bit(&rwlock->lock);
        }

        // Queue the writer and suspend
        sthread_queue_t *new_writer = malloc(sizeof(sthread_queue_t));
        new_writer->thread = sthread_self();
        new_writer->next = NULL;

        if (!rwlock->waiting_writers) {
            rwlock->waiting_writers = new_writer;
        } else {
            sthread_queue_t *temp = rwlock->waiting_writers;
            while (temp->next) temp = temp->next;
            temp->next = new_writer;
        }
        rwlock->waiting_writers_count++;

        sthread_suspend(); // Suspend the thread
    }
}

// Try to acquire the write lock (non-blocking)
int sthread_write_try_lock(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    // Try to acquire the lock without blocking
    if (test_and_set_bit(&rwlock->lock) == 0) {
        if (rwlock->active_readers == 0 && rwlock->active_writers == 0) {
            rwlock->active_writers++;
            clear_bit(&rwlock->lock);
            return 0;
        }
        clear_bit(&rwlock->lock);  // Release the lock
    }

    return EBUSY;  // Return immediately if the lock can't be acquired
}

// Release the write lock
int sthread_write_unlock(sthread_rwlock_t *rwlock)
{
    if (!rwlock) return -1;

    test_and_set_bit(&rwlock->lock); // Acquire the lock
    rwlock->active_writers--;

    // If there are writers waiting, prioritize them
    if (rwlock->waiting_writers) {
        sthread_queue_t *writer = rwlock->waiting_writers;
        rwlock->waiting_writers = rwlock->waiting_writers->next;
        rwlock->waiting_writers_count--;
        sthread_wake(writer->thread);
        free(writer);
    } else if (rwlock->waiting_readers) {
        // If no writers are waiting, wake all waiting readers
        while (rwlock->waiting_readers) {
            sthread_queue_t *reader = rwlock->waiting_readers;
            rwlock->waiting_readers = rwlock->waiting_readers->next;
            sthread_wake(reader->thread);
            free(reader);
        }
    }

    clear_bit(&rwlock->lock); // Release the lock
    return 0;
}
