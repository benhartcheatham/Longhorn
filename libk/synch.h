#ifndef _SYNCH_H
#define _SYNCH_H

/* includes */
#include "list.h"
#include "../kernel/thread.h"

/* defines */

/* structs */
struct semaphore {
    int val;
    list waiters;
} semaphore;

struct lock {
    struct semaphore s;
    struct thread *owner;
};

/* typedefs */
typedef semaphore semaphore;
typedef lock lock;

/* functions */

/* semaphore functions */
void semaphore_init(semaphore *s, int val);
void semaphore_down(semaphore *s);
void semaphore_up(semaphore *s);
int semaphore_try_down(semaphore *s);

/* lock functions */
void lock_init(lock *l, int val, struct thread *o);
void lock_acquire(lock *l);
void lock_release(lock *l);
int lock_try_acquire(lock *l);

#endif