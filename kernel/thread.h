/* Defines the threading subsystem */
#ifndef _THREAD_H
#define _THREAD_H

/* includes */
#include <stdint.h>
#include "isr.h"
#include "kalloc.h"
#include <list.h>

/* defines */
#define MAX_PNAME_LENGTH 12
#define MAX_TNAME_LENGTH 24
#define MAX_TID 512
#define STACK_SIZE PG_SIZE
#define THREAD_CUR() ((struct thread *) &get_running()->t)
#define PROC_CUR() ((struct process *) get_running()->p)
#define THREAD_MAGIC 0x33

/* structs */

// possible states of a thread 
enum thread_states {THREAD_READY, THREAD_RUNNING, THREAD_BLOCKED, THREAD_DYING, THREAD_TERMINATED};

struct thread {
    uint32_t *esp;  // the stack pointer of the thread *** MUST NOT BE MOVED FROM FIRST MEMBER ***
    uint32_t tid;   // the thread id of the thread
    uint32_t pid;   // the id of the process the thread is contained by
    uint8_t priority;   // the priority of the thread
    uint32_t ticks; // the amount of ticks the thread has been scheduled
    char name[MAX_PNAME_LENGTH + MAX_TNAME_LENGTH + 1]; // name of the thread
    enum thread_states state;   // state of the thread

    uint32_t child_num; // the location of the thread in the process thread array

    list_node node; // list node for ready and non-ready lists
    uint32_t magic;
};

// only for use in this file and thread.c
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
int thread_create(uint8_t priority, char *name, struct process *proc, uint32_t child_num, thread_function func, void *aux);
void thread_block(struct thread *thread);
void thread_unblock(struct thread *thread);
void thread_exit(int *ret);
int thread_kill(struct thread *thread);

/* thread "getter" functions */

/* gets the thread_info struct of the thread use either 
   the THREAD_CUR() or PROC_CUR() macros instead of this function */
static inline struct thread_info *get_running() {
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=g" (esp));
    // chop off last 12 bits to round to bottom of page
    esp = esp & (~(STACK_SIZE - 1));
    return (struct thread_info *) esp;
}

/* scheduling functions */
void thread_yield();
void timer_interrupt_handler(struct register_frame *r);
void finish_schedule();

#endif