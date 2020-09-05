#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "thread.h"
#include "kalloc.h"
#include "../libc/string.h"
#include "../libc/mem.h"

/* static data */
static struct list ready_threads;
static bool tids[MAX_TID];

/* data */
struct thread *init_t;
struct thread *current;
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

/* static functions */
static uint32_t allocate_tid();
static void set_thread_name(struct thread *t, char *name);
static void thread_execute(thread_function *func, void *aux);
static void schedule();

/* external functions */
extern void switch_thread(struct thread *current_thread, struct thread *next_thread);

void init_threads(struct process *init) {
    //init_t = &init->threads[0];
    init_t->child_num = 0;
    init_t->parent = init;
    init_t->state = THREAD_READY;
    init_t->tid = allocate_tid();
    asm volatile("mov %%esp, %0" : "=g" (init_t->regs.esp));
    init_t->regs.esp = (uint32_t *) ((uint32_t) init_t->regs.esp / PG_SIZE);
}

int thread_create(uint32_t priority, char *name, thread_function func, void *aux) {
    uint8_t *t = (uint8_t *) palloc();

    //setup the thread struct at the top of the page
    struct thread *thread = (struct thread *) t;
    thread->tid = allocate_tid();
    thread->state = THREAD_READY;
    set_thread_name(thread, name);
    thread->priority = priority;
    thread->node._struct = (void *) thread;

    t += PG_SIZE;

    t -= sizeof(struct thread_func_frame);
    struct thread_func_frame *f = (void *) t;
    f->eip = NULL;
    f->function = func;
    f->aux = (void *) aux;

    t -= sizeof(struct tail_frame);
    struct tail_frame *tf = (void *) t;
    tf->eip = (void (*) (void)) thread_execute;

    thread->regs.esp = (uint32_t *) t;

    list_insert(&ready_threads, &thread->node);
    thread->state = THREAD_READY;

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
}

static void thread_execute(thread_function *func, void *aux) {
    func(aux);
    thread_exit();
}

/* static functions */

static void schedule() {

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