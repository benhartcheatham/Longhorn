#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "thread.h"
#include "proc.h"
#include "kalloc.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../libc/mem.h"

/* defines */
#define MAX_THREAD_TICKS 8

/* static data */
//may also need an all list but im not sure
static struct list ready_threads;
static bool tids[MAX_TID];
static uint8_t thread_ticks = 0;

/* data */
struct thread *init_t;
struct thread *current;
struct thread *switch_temp;
struct thread *dying;


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

/* initializes threading */
void init_threads() {
    list_init(&ready_threads);

    init_t = &proc_get_running()->threads[0];
    init_t->node._struct = (void *) init_t;
    tids[0] = true;
    current = init_t;

    list_insert(&ready_threads, &init_t->node);
}

/* thread state functions */

/* creates a thread under the given parent process */
int thread_create(uint8_t priority, char *name, struct list_node *parent, struct thread *thread, thread_function func, void *aux) {
    uint8_t *s = (uint8_t *) palloc();

    //setup the thread struct at the top of the page
    thread->tid = allocate_tid();
    thread->state = THREAD_READY;
    sprintf(thread->name, "%s:%s", ((struct process *) parent->_struct)->name, name);
    thread->priority = priority;
    thread->node._struct = (void *) thread;
    thread->parent = parent;

    s += PG_SIZE;

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

    thread->esp = (uint32_t *) s;

    thread->state = THREAD_READY;
    list_insert(&ready_threads, &thread->node);

    return thread->tid;
}

/* blocks a thread */
void thread_block(struct thread *thread) {
    thread->state = THREAD_BLOCKED;
}

/* unblocks a thread and sets it to ready to run */
void thread_unblock(struct thread *thread) {
    thread->state = THREAD_READY;
}

/* function called at the end of the current thread's lifecycle */
void thread_exit() {
    current->state = THREAD_DYING;
    dying = thread_get_running();
    list_delete(&ready_threads, &current->node);

    schedule();
}

/* kills thread thread if owned by current process
   returns the tid of the killed thread if successful, -1 otherwise */
int thread_kill(struct thread *thread) {

    struct process *thread_parent = (struct process *) thread->parent->_struct;

    //only the parent process of the thread can kill it
    //while i like this thought, scheduling kind of screws it up
    //maybe only processes with a higher privilege can kill it?
    // if (proc_get_running() != thread_parent)
    //     return -1;
    
    if (thread->state == THREAD_READY)
        list_delete(&ready_threads, &thread->node);
        
    thread->state = THREAD_TERMINATED;
    pfree(thread->esp);
    
    thread_parent->num_live_threads--;
    return thread->tid;
}

/* thread "getter" functions */

/* returns a pointer to the running thread struct*/
struct thread *thread_get_running() {
    return current;
}

/* scheduling functions */

/* interrupt handler for the timer interrupt, also starts scheduling periodically */
void timer_interrupt_handler(struct register_frame *r __attribute__ ((unused))) {
    thread_ticks++;
    current->ticks++;

    if (thread_ticks % MAX_THREAD_TICKS == 0) {
        schedule();
    }
}

/* finishes up the scheduling process and updates thread state */
void finish_schedule() {
    //finish the part after we switch threads in schedule()

    //set old thread to ready
    current->state = THREAD_READY;

    //set current thread to running
    current = switch_temp;
    switch_temp = NULL;
    current->state = THREAD_RUNNING;

    //set running process
    proc_set_running();
    
    if (dying != NULL) {
        struct process *dying_parent = (struct process *) dying->parent->_struct;

        //only kill the parent process if this is its last thread
        if (proc_get_live_t_count(dying_parent) != 1)
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
    struct list_node *next = list_pop(&ready_threads);
    struct thread *next_thread = (struct thread *) next->_struct;
    
    list_insert_end(&ready_threads.tail, next);
    
    if (next == NULL || next->_struct == NULL || current == next_thread || next_thread->state != THREAD_READY)
        return;

    switch_temp = next_thread;
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