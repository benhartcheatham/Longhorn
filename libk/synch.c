/* This file contains all of the synchronization primitives of Longhorn
   As a note, when the OS becomes multicore, disabling interrupts won't work
   and I'll have to use atomic operations for the primitives */

#include "synch.h"
#include "list.h"
#include "../kernel/isr.h"
#include "../kernel/thread.h"

void semaphore_init(semaphore *s, int val) {
    s->val = val;
    list_init(&s->waiters);
}

void semaphore_down(semaphore *s) {
    disable_interrupts();
    while (s->value <= 0) {
        list_insert(&s->waiters, &thread_get_running()->node);
        thread_block(thread_get_running());

        enable_interrupts(); //this can be removed when thread_block is fully implemented
    }
        
    disable_interrupts(); //this can be removed when thread_block is fully implemented
    s->value--;
    enable_interrupts();
}

void semaphore_up(semaphore *s) {
    disable_interrupts();

    if (!list_isEmpty(&s->waiters)) {
        struct thread *t = list_pop(&s->waiters);
        thread_unblock(t);
    }

    s->value++;

    enable_interrupts();
}

int semaphore_try_down(semaphore *s) {
    disable_interrupts();

    if (s->value <= 0)
        return -1;
    else
        semaphore_down(s);
    
    return s->value;
}

void lock_init(lock *l, struct thread *o) {
    semaphore_init(l->s, 0);
    l->owner = o;
}

void lock_acquire(lock *l) {
    disable_interrupts();

    l->owner = thread_get_running();
    semaphore_down(l->s); //semaphore_down calls enable_interrupts as it finishes
}

void lock_release(lock *l) {
    disable_interrupts();

    l->owner = NULL;
    semaphore_up(l->s); //semaphore_up calls enable_interrupts as it finishes
}

int lock_try_acquire(lock *l) {
    disable_interrupts();

    if (l->s->value == 0)
        return -1;
    
    l->owner = thread_get_running();
    semaphore_down(l->s); //semaphore_down calls enable_interrupts as it finishes
}
