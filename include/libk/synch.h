/* Defines synchronization primitives for the kernel. */
#ifndef _SYNCH_H
#define _SYNCH_H

/* includes */
#include "list.h"
#include "../../kernel/thread.h"

/* defines */

/* structs */

struct spin_lock {
    int val;
    struct thread *owner;
};

struct semaphore {
    int val;
    struct spin_lock lock;
    list waiters;
};

/* typedefs */
typedef struct semaphore semaphore_t;
typedef struct spin_lock spin_lock_t;

/* functions */

/* semaphore functions */
int semaphore_init(semaphore_t *s, int val);
int semaphore_down(semaphore_t *s);
int semaphore_up(semaphore_t *s);
int semaphore_try_down(semaphore_t *s);

/* lock functions */
int spin_lock_init(spin_lock_t *sl);
int spin_lock_acquire(spin_lock_t *sl);
int spin_lock_release(spin_lock_t *sl);

#endif