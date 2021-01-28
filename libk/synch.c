/* Contains the synchronization primitives used in the kernel defined
 * in libk/synch.h. These primitives should be thread safe and safe to 
 * use in a multicore environment. */

#include <stddef.h>
#include <synch.h>
#include <atomic.h>
#include "../kernel/thread.h"


/* static functions */
static int __sdown(semaphore_t *s);

/* semaphore functions */

/* Initalizes a semaphore s with an inital value val
 * Returns an error code < 0 on failure and 0 on success */
int semaphore_init(semaphore_t *s, int val) {
    int ret = 0;

    if (s == NULL)
        return -LOCK_INIT_FAIL;
    
    s->val = val;

    ret = spin_lock_init(&s->lock);
    if (ret == 0)
        list_init(&s->waiters);

    return ret;
}

/* Calls the down operation on the semaphore s
 * If the semaphore has no resources left, the
 * calling thread is blocked until resources are
 * available
 * Returns an error code < 0 on failure and 0 on success */
int semaphore_down(semaphore_t *s) {
    // from: https://stackoverflow.com/questions/36094115/c-low-level-semaphore-implementation
    // while (1) {
    //     while (fetch_and_add(&s->val, 0) <= 0) {
    //         //do nothing
    //     }

    //     int old_val = fetch_and_add(&s->val, -1);
    //     if (old_val <= 0)
    //         break;
    //     else
    //         fetch_and_add(&s->val, 1);
    // }

    int ret = spin_lock_acquire(&s->lock);
    if (ret < 0)
        return ret;

    if (s->val > 0)
        s->val--;
    else
        ret = __sdown(s);

    if (ret < 0)
        return ret;
    
    ret = spin_lock_release(&s->lock);
    if (ret < 0)
        return ret;

    return LOCK_ACQ_SUCC;
}

/* Static function that does the blocking of the
 * thread trying to call down if the semaphore has
 * no resources available */
static int __sdown(semaphore_t *s) {
    list_insert(&s->waiters, &s->lock.owner->node);

    int ret = 0;
    ret = spin_lock_release(&s->lock);

    if (ret < 0)
        return ret;

    thread_block(THREAD_CUR());

    spin_lock_acquire(&s->lock);

    return LOCK_ACQ_SUCC;
}

/* Calls the up operation on a semaphore
 * If there are other threads waiting on this semaphore,
 * one is unblocked to be scheduled, otherwise the
 * value of the semaphore is incremented 
 * Returns an error code < 0 on failure and 0 on success */
int semaphore_up(semaphore_t *s) {
    int ret = 0;

    ret = spin_lock_acquire(&s->lock);
    if (ret < 0)
        return ret;

    if (!list_isEmpty(&s->waiters)) {
        struct thread *next = LIST_ENTRY(list_pop(&s->waiters), struct thread, node);
        thread_unblock(next);
    } else
        s->val++;
    
    ret = spin_lock_release(&s->lock);
    if (ret < 0)
        return ret;

    return LOCK_REL_SUCC;
}

/* Tries to call down on the semaphore s
 * Returns an error code < 0 if the attempt
 * fails and 0 on success */
int semaphore_try_down(semaphore_t *s) {

    int ret = spin_lock_acquire(&s->lock);
    if (ret < 0)
        return ret;

    if (s->val > 0)
        s->val--;
    else {
        ret = -LOCK_ACQ_FAIL;
        spin_lock_release(&s->lock);

        return ret;
    }
    
    ret = spin_lock_release(&s->lock);
    if (ret < 0)
        return ret;

    return LOCK_ACQ_SUCC;
}


/* spin lock functions */

/* Initializes an unlocked spin lock with owner thread o 
 * Returns _LOCK_INIT_FAIL if o is null and 0 on
 * a successful initialization */
int spin_lock_init(spin_lock_t *sl) {
    if (sl == NULL)
        return -LOCK_INIT_FAIL;
    
    sl->val = 0;
    sl->owner = NULL;

    return LOCK_INIT_SUCC;
}

/* Acquires a spin lock
 * The thread will busy wait until the thread acquires the lock 
 * Returns -LOCK_ACQ_FAIL if the lock value is corrupted and
 * 0 if the lock is acquired */
int spin_lock_acquire(spin_lock_t *sl) {
    int val = sl->val;

    // check if the value is either one or zero
    // the actual status of the lock doesn't matter so we
    // don't have to check atomically
    if (!(val == 0 || val == 1))
        return -LOCK_ACQ_FAIL;

    while (test_and_set(&sl->val) > 0) {
        // do nothing
    }

    return LOCK_ACQ_SUCC;
}

/* Releases a spin lock
 * Returns -LOCK_REL_FAIL if the lock value is corrupted
 * or the calling thread does not own the lock and 0
 * if the release call is successfull */
int spin_lock_release(spin_lock_t *sl) {

    // if the lock had a bad value (it's already free),
    // just return an error code and don't attempt to fix
    // it. Other functions shouldn't allow the lock to 
    // be used after the error

    // this has to be commented out if its used with kmalloc
    // because kmalloc isn't initialized yet
    //if (sl == NULL || sl->owner != thread_get_running())
        //return -LOCK_REL_FAIL;
    
    if (fetch_and_add(&sl->val, -1) != 1)
        return -LOCK_REL_FAIL;

    return -LOCK_REL_SUCC;
}
