#ifndef _THREAD_H
#define _THREAD_H

/* includes */
#include <stdint.h>
#include "isr.h"
#include "../libk/list.h"

/* defines */
#define MAX_PNAME_LENGTH 12
#define MAX_TNAME_LENGTH 24
#define MAX_TID 512
#define THREAD_MAGIC 0x5f5f5f5fu

/* structs */
enum thread_states {THREAD_READY, THREAD_RUNNING, THREAD_BLOCKED, THREAD_DYING, THREAD_TERMINATED};

struct thread {
    uint32_t *esp;
    uint32_t tid;
    uint8_t priority;
    uint32_t ticks;
    char name[MAX_PNAME_LENGTH + MAX_TNAME_LENGTH + 1];
    enum thread_states state;

    //have to use a list_node * instead of a process * because of recursive includes that I couldn't see an
    //easy fix to. Probably want to come back and fix this
    struct list_node *parent;
    uint32_t child_num;

    struct list_node node;
};


/* typedefs */
typedef void (thread_function) (void *aux);

/* functions */

/* initialization functions */
void init_threads();

/* thread state functions */
int thread_create(uint8_t priority, char *name, struct list_node *parent, struct thread *thread, thread_function func, void *aux);
void thread_block(struct thread *thread);
void thread_unblock(struct thread *thread);
void thread_exit();
void thread_yield();
int thread_kill(struct thread *thread);

/* thread "getter" functions */
struct thread *thread_get_running();

/* scheduling functions */
void timer_interrupt_handler(struct register_frame *r);
void finish_schedule();

#endif