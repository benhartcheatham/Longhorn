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
#define MAX_THREAD_TICKS 24

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
static void set_thread_name(struct thread *t, char *name);
static void thread_execute(thread_function *func, void *aux);
static void schedule();
extern void first_switch_entry();

/* external functions */
extern void switch_threads(struct thread *current_thread, struct thread *next_thread);

/* initialization functions */

void init_threads() {
    list_init(&ready_threads);

    init_t = &proc_get_running()->threads[0];
    init_t->node._struct = (void *) init_t;
    tids[0] = true;
    current = init_t;

    list_insert(&ready_threads, &init_t->node);
}

/* thread state functions */

int thread_create(uint8_t priority, char *name, struct thread *thread, thread_function func, void *aux) {
    uint8_t *s = (uint8_t *) palloc();

    //setup the thread struct at the top of the page
    thread->tid = allocate_tid();
    thread->state = THREAD_READY;
    set_thread_name(thread, name);
    thread->priority = priority;
    thread->node._struct = (void *) thread;
    thread->parent = &proc_get_running()->node;

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

void thread_block(struct thread *thread) {
    thread->state = THREAD_BLOCKED;
}

void thread_unblock(struct thread *thread) {
    thread->state = THREAD_READY;
}

void thread_exit() {
    current->state = THREAD_DYING;
    list_delete(&ready_threads, &current->node);

    schedule();
}

/* kills thread thread if owned by current process
   returns the tid of the killed thread if successful, -1 otherwise */
int thread_kill(struct thread *thread) {
    //only the parent process of the thread can kill it
    if (proc_get_running() != (struct process *) thread->parent->_struct)
        return -1;
    
    struct thread *temp = &((struct process *) thread->parent->_struct)->threads[thread->child_num];

    if (temp->state == THREAD_READY)
        list_delete(&ready_threads, &temp->node);
        
    temp->state = THREAD_TERMINATED;
    pfree(temp->esp);
    
    return temp->tid;
}

static void thread_execute(thread_function func, void *aux) {
    //have to reenable interrupts since there isn't a guaruntee we returned to the irq handler
    enable_interrupts();
    func(aux);
    thread_exit();
}

/* scheduling functions */

void timer_interrupt_handler(struct register_frame *r __attribute__ ((unused))) {
    thread_ticks++;

    if (thread_ticks >= MAX_THREAD_TICKS) {
        schedule();
    }
}

void finish_schedule() {
    //finish the part after we switch threads in schedule()
    current = switch_temp;
    switch_temp = NULL;
    return;
}

/* static functions */

static void schedule() {
    struct list_node *next = list_pop(&ready_threads);
    if (next == NULL || next->_struct == NULL || current == (struct thread *) next->_struct)
        return;

    list_insert_end(&ready_threads.tail, next);

    switch_temp = (struct thread *) next->_struct;
    switch_threads(current, (struct thread *) next->_struct);
    current = switch_temp;
    switch_temp = NULL;
}

static uint32_t allocate_tid() {
    int i;
    for (i = 0; i < MAX_TID; i++)
        if (tids[i] == false) {
            tids[i] = true;
            return i;
        }
    
    return MAX_TID + 1;
}

static void set_thread_name(struct thread *t, char *name) {
    if (strlen(name) < MAX_TNAME_LENGTH) {
        memcpy(t->name, name, MAX_TNAME_LENGTH);
        t->name[strlen(name) + 1] = '\0';
    }
}