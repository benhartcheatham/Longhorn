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
#include <stdio.h>
#include <synch.h>

/* defines */
#define MAX_THREAD_TICKS 8

/* static data */
static struct list ready_threads;
static struct list blocked_threads;
static struct list dying_threads;
static struct thread *idle_t;
static bool tids[MAX_TID];
static uint8_t thread_ticks = 0;

/* data */

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
static void idle(void *aux);

/* external functions */
extern void switch_threads(struct thread *current_thread, struct thread *next_thread);

/* initialization functions */

/* initializes threading */
void init_threads(struct process *init) {
    list_init(&ready_threads);
    list_init(&blocked_threads);
    list_init(&dying_threads);

    thread_create(0, "idle", init, &init->threads[0], idle, NULL);
    idle_t = init->threads[0];

}

/* thread state functions */

/* creates a thread under the given parent process */
int thread_create(uint8_t priority, char *name, struct process *parent, struct thread **sthread, thread_function func, void *aux) {
    uint8_t *s = (uint8_t *) palloc_mult(STACK_SIZE / PG_SIZE);

    if (s == NULL)
        return -1;
    
    // setup the thread struct at the bottom of the page (lowest addr)
    struct thread_info *ti = (struct thread_info *) s;

    ti->t.tid = allocate_tid();

    // if the max amount of threads on the system is already met don't allow creation
    if (ti->t.tid == MAX_TID + 1) {
        kfree((void *) s);
        return -1;
    }

    ti->t.state = THREAD_READY;
    sprintf(ti->t.name, "%s", name);
    ti->t.priority = priority;
    ti->t.pid = parent->pid;

    // add a pointer to the parent process after thread struct
    ti->p = parent;
    *sthread = &ti->t;

    s += STACK_SIZE;

    // setup arguments thread_execute
    s -= sizeof(struct thread_func_frame);
    struct thread_func_frame *f = (struct thread_func_frame *) s;
    f->eip = NULL;
    f->function = func;
    f->aux = (void *) aux;

    // setup to call thread_execute
    s -= sizeof(struct tail_frame);
    struct tail_frame *tf = (struct tail_frame *) s;
    tf->eip = (void (*) (void)) thread_execute;

    // setup for the first switch of a thread
    s -= sizeof(struct stack_frame);
    struct stack_frame *sf = (struct stack_frame *) s;
    sf->eip = first_switch_entry;
    sf->ebp = 0;

    ti->t.esp = (uint32_t *) s;

    ti->t.state = THREAD_READY;

    disable_interrupts();
    list_insert(&ready_threads, &ti->t.node);
    enable_interrupts();

    return ti->t.tid;
}

/* blocks a thread */
void thread_block(struct thread *thread) {
    disable_interrupts();

    struct list_node *node = list_delete(&ready_threads, &thread->node);

    // running threads aren't in the ready list, so we
    // need to account for that case
    if (node == NULL && thread->state != THREAD_RUNNING) {
        enable_interrupts();
        return;
    }
    
    thread->state = THREAD_BLOCKED;
    list_insert(&blocked_threads, node);

    enable_interrupts();

    schedule();
}

/* unblocks a thread and sets it to ready to run */
void thread_unblock(struct thread *thread) {
    disable_interrupts();

    struct list_node *node = list_delete(&blocked_threads, &thread->node);

    if (node == NULL) {
        enable_interrupts();
        return;
    }
    
    thread->state = THREAD_READY;
    list_insert(&ready_threads, node);
    enable_interrupts();
}

/* function called at the end of the current thread's lifecycle */
void thread_exit() {
    struct thread *t = THREAD_CUR();

    disable_interrupts();

    list_delete(&ready_threads, &t->node);
    t->state = THREAD_DYING;
    list_insert(&dying_threads, &t->node);

    enable_interrupts();

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
        disable_interrupts();

        list_delete(&ready_threads, &thread->node);

        thread_parent->threads[thread->child_num] = NULL;
        uint32_t temp = (uint32_t) thread->esp & (~(STACK_SIZE - 1));
        pfree((void *) temp);
    
        thread_parent->num_live_threads--;
        enable_interrupts();

    } else {
        disable_interrupts();
        
        list_delete(&blocked_threads, &thread->node);

        thread_parent->threads[thread->child_num] = NULL;
        uint32_t temp = (uint32_t) thread->esp & (~(STACK_SIZE - 1));
        pfree((void *) temp);

        thread_parent->num_live_threads--;
        enable_interrupts();
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

    enable_interrupts();
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
    disable_interrupts();

    list_node *next = list_pop(&ready_threads);
    struct thread *next_thread = NULL;
    struct thread *current = THREAD_CUR();

    // if we have no ready threads, we schedule the idle thread
    // otherwise we just pull one of the top
    if (next == NULL) {
        next_thread = idle_t;
        idle_t->state = THREAD_READY;
    
    } else {
        next_thread = LIST_ENTRY(next, struct thread, node);
    }

    if (current->state == THREAD_RUNNING)
        current->state = THREAD_READY;

    if (current->state == THREAD_DYING)
        list_insert(&dying_threads, &current->node);
    
    if (current->state == THREAD_READY)
        list_insert_end(&ready_threads.tail, &current->node);

    if (current == next_thread || next_thread->state != THREAD_READY)
        return;

    switch_threads(current, next_thread);

    finish_schedule();
}

/* function that the init thread runs after interrupts are enabled */
static void idle(void *aux __attribute__ ((unused))) {
    while (1) {
        thread_block(THREAD_CUR());
    };
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

/* testing functions */
size_t num_threads() {
    return list_size(&ready_threads) + list_size(&blocked_threads);
}