/* May want to make the stack limited to a page or two and put them at
  a specified interval like in linux. This would allow getting rid of
  the current pointer and just doing some quick math with esp to get
  the running thread.
  
  For synchronization, I think synchronizing around the to thread lists
  is correct, but to get around schedule blocking, it should try to acquire the 
  semaphore/lock and then schedule the thread that holds the lock if it can't
  
  Priority scehduling is also something I will want to implement now */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "thread.h"
#include "proc.h"
#include "kalloc.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libk/synch.h"

/* defines */
#define MAX_THREAD_TICKS 8

/* static data */
static struct list ready_threads;
static struct list blocked_threads;
static struct list dying_threads;
static bool tids[MAX_TID];
static uint8_t thread_ticks = 0;

/* data */
struct thread *init_t;
struct thread *switch_temp;
struct spin_lock r_lock;
struct spin_lock b_lock;

/* structs */
struct thread_func_frame {
    void *eip;
    thread_function *function;
    void *aux;
};

struct tail_frame {
    void (*eip) (void);
};

struct stack_frame {
    uint32_t ebp, ebx, edi, esi;
    void (*eip) (void);
};

/* static functions */
static uint32_t allocate_tid();
static void thread_execute(thread_function *func, void *aux);
static void schedule();
extern void first_switch_entry();

/* external functions */
extern void switch_threads(struct thread *current_thread, struct thread *next_thread);

/* initialization functions */

/* function that the init thread runs after interrupts are enabled */
static void __init(void *aux __attribute__ ((unused))) {
    while (1) {
        thread_yield();
        // thread_block(THREAD_CUR());
    };
}

/* initializes threading */
void init_threads(struct process *init) {
    list_init(&ready_threads);
    list_init(&blocked_threads);
    list_init(&dying_threads);
    spin_lock_init(&r_lock);
    spin_lock_init(&b_lock);

    thread_create(0, "init", init, &init->threads[0], __init, NULL);
    init_t = init->threads[0];

}

/* thread state functions */

/* creates a thread under the given parent process */
int thread_create(uint8_t priority, char *name, struct process *parent, struct thread **sthread, thread_function func, void *aux) {
    uint8_t *s = (uint8_t *) palloc_mult(STACK_SIZE / PG_SIZE);

    //setup the thread struct at the bottom of the page (lowest addr)
    struct thread_info *ti = (struct thread_info *) s;

    ti->t.tid = allocate_tid();
    ti->t.state = THREAD_READY;
    sprintf(ti->t.name, "%s", name);
    ti->t.priority = priority;
    ti->t.pid = parent->pid;

    //add a pointer to the parent process after thread struct
    ti->p = parent;
    *sthread = &ti->t;

    s += STACK_SIZE;

    //setup arguments thread_execute
    s -= sizeof(struct thread_func_frame);
    struct thread_func_frame *f = (struct thread_func_frame *) s;
    f->eip = NULL;
    f->function = func;
    f->aux = (void *) aux;

    //setup to call thread_execute
    s -= sizeof(struct tail_frame);
    struct tail_frame *tf = (struct tail_frame *) s;
    tf->eip = (void (*) (void)) thread_execute;

    //setup for the first switch of a thread
    s -= sizeof(struct stack_frame);
    struct stack_frame *sf = (struct stack_frame *) s;
    sf->eip = first_switch_entry;
    sf->ebp = 0;

    ti->t.esp = (uint32_t *) s;

    ti->t.state = THREAD_READY;

    // if we can't acquire, something is wrong and the thread shouldn't be created
    if (spin_lock_acquire(&r_lock) != LOCK_ACQ_SUCC) {
        uint32_t temp = (uint32_t) s & (~(STACK_SIZE - 1));
        kfree((void *) temp);
        return -1;
    }

    list_insert(&ready_threads, &ti->t.node);
    spin_lock_release(&r_lock);

    return ti->t.tid;
}

/* blocks a thread */
void thread_block(struct thread *thread) {
    if (spin_lock_acquire(&r_lock) != LOCK_ACQ_SUCC)
        return;
    
    struct list_node *node = list_delete(&ready_threads, &thread->node);
    spin_lock_release(&r_lock);

    // failing this check for some treason,
    // but doesn't if i insert next instead of
    // &current->node into ready_threads in schedule
    if (node == NULL)
        return;
    
    // if we can't acquire the blocked list lock,
    // reinsert into the ready queue and return
    if (spin_lock_acquire(&b_lock) != LOCK_ACQ_SUCC) {
        if (spin_lock_acquire(&r_lock) != LOCK_ACQ_SUCC)
            return;

        list_delete(&ready_threads, node);
        spin_lock_release(&r_lock);
        return;
    }
    
    thread->state = THREAD_BLOCKED;
    list_insert(&blocked_threads, node);
    spin_lock_release(&b_lock);

    schedule();
}

/* unblocks a thread and sets it to ready to run */
void thread_unblock(struct thread *thread) {
    if (spin_lock_acquire(&b_lock) != LOCK_ACQ_SUCC)
        return;

    struct list_node *node = list_delete(&blocked_threads, &thread->node);
    spin_lock_release(&b_lock);

    if (node == NULL)
        return;
    
    // if we can't acquire the ready queue lock,
    // reinsert into the blocked list and return
    if (spin_lock_acquire(&r_lock) != LOCK_ACQ_SUCC) {
        if (spin_lock_acquire(&b_lock) != LOCK_ACQ_SUCC)
            return;

        list_delete(&blocked_threads, node);
        spin_lock_release(&b_lock);
        return;
    }
    
    thread->state = THREAD_READY;
    list_insert(&ready_threads, node);
    spin_lock_release(&r_lock);
}

/* function called at the end of the current thread's lifecycle */
void thread_exit() {
    struct thread *t = THREAD_CUR();

    if (spin_lock_acquire(&r_lock) != LOCK_ACQ_SUCC)
        return;
    
    list_delete(&ready_threads, &t->node);
    t->state = THREAD_DYING;
    list_insert(&dying_threads, &t->node);
    spin_lock_release(&r_lock);

    schedule();
}

/* kills thread thread if owned by current process
   returns the tid of the killed thread if successful, -1 otherwise 
   
   THIS DOESN'T RELEASE THE LOCKS HELD BY THE THREAD, NEEDS TO BE UPDATED */
int thread_kill(struct thread *thread) {
    if (thread == NULL)
        return -1;
    
    struct process *thread_parent = get_thread_proc(thread);

    if (thread->state == THREAD_READY || thread->state == THREAD_RUNNING) {
        if (spin_lock_acquire(&r_lock) != LOCK_ACQ_SUCC)
            return -1;

        list_delete(&ready_threads, &thread->node);

        thread_parent->threads[thread->child_num] = NULL;
        uint32_t temp = (uint32_t) thread->esp & (~(STACK_SIZE - 1));
        pfree((void *) temp);
    
        thread_parent->num_live_threads--;
        spin_lock_release(&r_lock);

    } else {
        if (spin_lock_acquire(&b_lock) != LOCK_ACQ_SUCC)
            return -1;
        
        list_delete(&blocked_threads, &thread->node);

        thread_parent->threads[thread->child_num] = NULL;
        uint32_t temp = (uint32_t) thread->esp & (~(STACK_SIZE - 1));
        pfree((void *) temp);

        thread_parent->num_live_threads--;
        spin_lock_release(&b_lock);
    }

    return thread->tid;
}

/* scheduling functions */

void thread_yield() {
    schedule();
}

/* interrupt handler for the timer interrupt, also starts scheduling periodically */
void timer_interrupt_handler(struct register_frame *r __attribute__ ((unused))) {
    thread_ticks++;
    THREAD_CUR()->ticks++;

    if (thread_ticks % MAX_THREAD_TICKS == 0) {
        schedule();
    }
}

/* finishes up the scheduling process and updates thread state */
void finish_schedule() {
    //finish the part after we switch threads in schedule()

    //set current thread to running
    struct thread *cur = THREAD_CUR();
    cur->state = THREAD_RUNNING;
    proc_set_active_thread(PROC_CUR(), cur->child_num);
    
    if (list_isEmpty(&dying_threads) == false) {
        struct thread *dying = LIST_ENTRY(list_pop(&dying_threads), struct thread, node);
        struct process *dying_parent = get_thread_proc(dying);

        //only kill the parent process if this is its last thread
        if (proc_get_live_t_count(dying_parent) > 1)
            thread_kill(dying);
        else
            proc_exit(dying_parent);
        
        dying = NULL;
    }
}

/* static functions */

/* executes the function the thread is created to do and kills the thread when done */
static void thread_execute(thread_function func, void *aux) {
    //have to reenable interrupts since there isn't a guaruntee we returned to the irq handler
    asm volatile("sti");
    func(aux);
    thread_exit();
}

/* schedules threads */
static void schedule() {
    list_node *next = list_pop(&ready_threads);
    struct thread *next_thread = LIST_ENTRY(next, struct thread, node);
    struct thread *current = THREAD_CUR();

    if (current->state == THREAD_RUNNING)
        current->state = THREAD_READY;

    if (current->state == THREAD_DYING)
        list_insert(&dying_threads, &current->node);
    
    if (current->state == THREAD_READY)
        list_insert_end(&ready_threads.tail, &current->node);
    
    if (next == NULL || current == next_thread || next_thread->state != THREAD_READY)
        return;

    switch_threads(current, next_thread);

    finish_schedule();
}

/* allocates a thread id for when a thread is being created */
static uint32_t allocate_tid() {
    int i;
    for (i = 0; i < MAX_TID; i++)
        if (tids[i] == false) {
            tids[i] = true;
            return i;
        }
    
    return MAX_TID + 1;
}