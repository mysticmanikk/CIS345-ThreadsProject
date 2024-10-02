/*
 * NAME, etc.
 *
 * sync.h
 */

#ifndef _STHREAD_SYNC_H_
#define _STHREAD_SYNC_H_


// Define the queue structure for managing waiting threads
typedef struct sthread_queue {
    sthread_t thread;               // Thread identifier
    struct sthread_queue *next;     // Pointer to the next thread in the queue
} sthread_queue_t;

// Define the read-write lock structure
struct sthread_rwlock_struct {
    int active_readers;              // Count of currently active readers
    int active_writers;              // Count of currently active writers
    int waiting_writers_count;       // Count of writers waiting for the lock
    sthread_queue_t *waiting_readers; // Queue for waiting readers
    sthread_queue_t *waiting_writers; // Queue for waiting writers
    volatile unsigned long lock;     // Simple lock for synchronization
};

typedef struct sthread_rwlock_struct sthread_rwlock_t;

int sthread_rwlock_init(sthread_rwlock_t *rwlock);
int sthread_rwlock_destroy(sthread_rwlock_t *rwlock);
int sthread_read_lock(sthread_rwlock_t *rwlock);
int sthread_read_try_lock(sthread_rwlock_t *rwlock);
int sthread_read_unlock(sthread_rwlock_t *rwlock);
int sthread_write_lock(sthread_rwlock_t *rwlock);
int sthread_write_try_lock(sthread_rwlock_t *rwlock);
int sthread_write_unlock(sthread_rwlock_t *rwlock);

#endif
