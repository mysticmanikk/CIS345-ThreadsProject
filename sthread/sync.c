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

int sthread_rwlock_init(sthread_rwlock_t *rwlock)
{	/*FILL ME IN */
    rwlock->writer = 0;
    rwlock->readers = 0;
    rwlock->waiting_writers = 0;
    sthread_queue_init(&rwlock->read_queue);
    sthread_queue_init(&rwlock->write_queue);
    return 0;
}


int sthread_rwlock_destroy(sthread_rwlock_t *rwlock)
{	/*FILL ME IN */
    if (rwlock->writer || rwlock->readers > 0) {
        // Cannot destroy a lock that is still in use
        return -1;
    }
    sthread_queue_destroy(&rwlock->read_queue);
    sthread_queue_destroy(&rwlock->write_queue);
    return 0;
}


int sthread_read_lock(sthread_rwlock_t *rwlock)
{	/*FILL ME IN */
    while (1) {
        if (rwlock->writer || rwlock->waiting_writers) {
            // Writer is active or writers are waiting, suspend reader
            sthread_queue_add(&rwlock->read_queue, sthread_self());
            sthread_suspend();
        } else {
            // Increment the readers counter and return
            __sync_fetch_and_add(&rwlock->readers, 1);
            break;
        }
    }
    return 0;
}


int sthread_read_try_lock(sthread_rwlock_t *rwlock)
{	/*FILL ME IN */
    if (rwlock->writer || rwlock->waiting_writers) {
        // If a writer is active or waiting, fail to acquire the lock
        return -1;
    }
    __sync_fetch_and_add(&rwlock->readers, 1);
    return 0;
}

int sthread_read_unlock(sthread_rwlock_t *rwlock)
{	/*FILL ME IN */
    __sync_fetch_and_sub(&rwlock->readers, 1);
    if (rwlock->readers == 0 && !sthread_queue_empty(&rwlock->write_queue)) {
        // No readers left, wake a writer
        sthread_wake(sthread_queue_remove(&rwlock->write_queue));
    }
    return 0;
}


int sthread_write_lock(sthread_rwlock_t *rwlock)
{	/*FILL ME IN */
    __sync_fetch_and_add(&rwlock->waiting_writers, 1);
    while (1) {
        if (rwlock->readers == 0 && !rwlock->writer) {
            // No active readers or writers, acquire the write lock
            rwlock->writer = 1;
            __sync_fetch_and_sub(&rwlock->waiting_writers, 1);
            break;
        } else {
            // Suspend writer if conditions are not met
            sthread_queue_add(&rwlock->write_queue, sthread_self());
            sthread_suspend();
        }
    }
    return 0;
}

int sthread_write_try_lock(sthread_rwlock_t *rwlock)
{
	/*FILL ME IN */
    if (rwlock->readers == 0 && !rwlock->writer) {
        rwlock->writer = 1;
        return 0;
    }
    return -1; // Lock could not be acquired
}


int sthread_write_unlock(sthread_rwlock_t *rwlock)
{
        /* FILL ME IN! */
    rwlock->writer = 0;
    if (!sthread_queue_empty(&rwlock->write_queue)) {
        // Wake up the next writer if any are waiting
        sthread_wake(sthread_queue_remove(&rwlock->write_queue));
    } else {
        // Otherwise, wake up all waiting readers
        while (!sthread_queue_empty(&rwlock->read_queue)) {
            sthread_wake(sthread_queue_remove(&rwlock->read_queue));
        }
    }
    return 0;
}


