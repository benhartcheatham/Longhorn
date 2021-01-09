#ifndef _THREAD_H
#define _THREAD_H

/* includes */
#include <stdint.h>
#include "isr.h"
#include "kalloc.h"
#include "../libk/list.h"

/* defines */
#define MAX_PNAME_LENGTH 12
#define MAX_TNAME_LENGTH 24
#define MAX_TID 512
#define THREAD_MAGIC 0x5f5f5f5fu
#define STACK_SIZE PG_SIZE
#define THREAD_CUR() ((struct thread *) get_running())
#define PROC_CUR() ((struct process *) (((char *) get_running()) + offsetof(struct thread_info, p)))

/* structs */
enum thread_states {THREAD_READY, THREAD_RUNNING, THREAD_BLOCKED, THREAD_DYING, THREAD_TERMINATED};

struct thread {
    uint32_t *esp;
    uint32_t tid;
    uint32_t pid;
    uint8_t priority;
    uint32_t ticks;
    char name[MAX_PNAME_LENGTH + MAX_TNAME_LENGTH + 1];
    enum thread_states state;

    //have to use a list_node * instead of a process * because of recursive includes that I couldn't see an
    //easy fix to. Probably want to come back and fix this
    uint32_t child_num;

    list_node node;
};

struct __attribute__ ((packed)) thread_info {
    struct thread t;
    struct process *p;
};

/* typedefs */
typedef void (thread_function) (void *aux);
typedef struct thread thread_t;
typedef struct thread_info thread_info_t;

/* functions */

/* initialization functions */
void init_threads(struct process *init_p);

/* thread state functions */
int thread_create(uint8_t priority, char *name, struct process *parent, struct thread **sthread, thread_function func, void *aux);
void thread_block(struct thread *thread);
void thread_unblock(struct thread *thread);
void thread_exit();
void thread_yield();
int thread_kill(struct thread *thread);

/* thread "getter" functions */
static inline struct thread_info *get_running() {
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=g" (esp));
    // chop off last 12 bits to round to bottom of page
    esp = esp & (~(PG_SIZE - 1));
    return (struct thread_info *) esp;
}

/* scheduling functions */
void timer_interrupt_handler(struct register_frame *r);
void finish_schedule();

#endif