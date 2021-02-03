#ifndef _SYNCH_H
#define _SYNCH_H

/* includes */
#include "list.h"
#include "../../kernel/thread.h"

/* defines */
#define LOCK_INIT_FAIL 1
#define LOCK_INIT_SUCC 0
#define LOCK_ACQ_FAIL 2
#define LOCK_ACQ_SUCC 0
#define LOCK_REL_FAIL 3
#define LOCK_REL_SUCC 0

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

struct lock {
    struct semaphore lock;
    struct thread *owner;
};

/* typedefs */
typedef struct semaphore semaphore_t;
typedef struct spin_lock spin_lock_t;
typedef struct lock lock_t;

/* functions */

/* semaphore functions */
int semaphore_init(semaphore_t *s, int val);
int semaphore_down(semaphore_t *s);
int semaphore_up(semaphore_t *s);
int semaphore_try_down(semaphore_t *s);

/* spinlock functions */
int spin_lock_init(spin_lock_t *sl);
int spin_lock_acquire(spin_lock_t *sl);
int spin_lock_release(spin_lock_t *sl);

/* lock functions */
int lock_init(lock_t *l);
int lock_acquire(lock_t *l);
int lock_release(lock_t *l);
int lock_try_acquire(lock_t *l);

#endif