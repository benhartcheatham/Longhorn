/* Contains the synchronization primitives used in the kernel defined
 * in libk/synch.h. These primitives should be thread safe and safe to 
 * use in a multicore environment. */

/* includes */
#include <stddef.h>
#include <synch.h>
#include <atomic.h>
#include <kerrors.h>
#include "../kernel/thread.h"

/* defines */

/* globals */

/* prototypes */
static int __sdown(semaphore_t *s);

/* functions */

/* semaphore functions */

/** initalizes a semaphore s with an inital value val
 * 
 * @param s: semaphore to init
 * @param val: initial value of s
 * 
 * @return error code < 0 on failure and 0 on success */
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

/** calls the down operation on the semaphore s
 * If the semaphore has no resources left, the
 * calling thread is blocked until resources are
 * available
 * 
 * @param s: semaphore to down on
 * 
 * @return error code < 0 on failure and 0 on success */
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

/** static function that does the blocking of the
 * thread trying to call down if the semaphore has
 * no resources available 
 * 
 * @param s: semaphore to down on
 * 
 * @return LOCK_ACQ_SUCC on success, error code < 0 otherwise
 */
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

/** calls the up operation on a semaphore
 * if there are other threads waiting on this semaphore,
 * one is unblocked to be scheduled, otherwise the
 * value of the semaphore is incremented 
 * 
 * @param s: semaphore to up
 * 
 * @return error code < 0 on failure and 0 on success */
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

/** tries to call down on the semaphore s
 * 
 * @param s: semaphore to try to down
 * 
 * @return error code < 0 on failure and 0 on success */
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

/** initializes an unlocked spin lock
 *
 * @param sl: spinlock to initialize
 * 
 * @return -LOCK_INIT_FAIL on failure, LOCK_INIT_SUCC otherwise */
int spin_lock_init(spin_lock_t *sl) {
    if (sl == NULL)
        return -LOCK_INIT_FAIL;
    
    sl->val = 0;
    sl->owner = NULL;

    return LOCK_INIT_SUCC;
}

/** acquires a spin lock
 * the thread will busy wait until the thread acquires the lock 
 * 
 * @param sl: spinlock to acquire
 * 
 * @return -LOCK_ACQ_FAIL on failure, LOCK_ACQ_SUCC otherwise */
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

/** releases a spin lock
 * 
 * @param sl: spinlock to release
 * 
 * @return -LOCK_REL_FAIL on failure, LOCK_REL_SUCC otherwise*/
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
