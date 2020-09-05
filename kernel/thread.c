#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "thread.h"
#include "proc.h"
#include "kalloc.h"
#include "../libc/string.h"
#include "../libc/mem.h"

/* static data */
//may also need an all list but im not sure
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

void init_threads() {
    list_init(&ready_threads);

    init_t = &proc_get_running()->threads[0];
    tids[0] = true;
    current = init_t;

    list_insert(&ready_threads, &init_t->node);
}

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

    s -= sizeof(struct thread_func_frame);
    struct thread_func_frame *f = (void *) s;
    f->eip = NULL;
    f->function = func;
    f->aux = (void *) aux;

    s -= sizeof(struct tail_frame);
    struct tail_frame *tf = (void *) s;
    tf->eip = (void (*) (void)) thread_execute;

    thread->regs.esp = (uint32_t *) s;

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