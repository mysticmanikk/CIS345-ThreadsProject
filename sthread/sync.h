/*
 * NAME, etc.
 *
 * sync.h
 */

#ifndef _STHREAD_SYNC_H_
#define _STHREAD_SYNC_H_


typedef struct sthread_rwlock_struct {
    volatile unsigned long writer;          // Flag to indicate if a writer is active
    volatile unsigned long readers;         // Counter for active readers
    volatile unsigned long waiting_writers; // Counter for waiting writers
    sthread_queue_t read_queue;             // Queue for waiting readers
    sthread_queue_t write_queue;            // Queue for waiting writers
} sthread_rwlock_t;

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
