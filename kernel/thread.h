#ifndef _THREAD_H
#define _THREAD_H

/* includes */
#include <stdint.h>
#include "../libk/list.h"

/* defines */
#define MAX_TNAME_LENGTH 24
#define MAX_TID 512

/* structs */
struct state_regs {
    //registers in the order of a pushad
    uint32_t eax, ecx, edx, ebx;
    uint32_t *esp, ebp, esi, edi;
};

enum thread_states {THREAD_READY, THREAD_BLOCKED, THREAD_DYING, 
                            THREAD_SUSPENDED, THREAD_TERMINATED};

struct thread {
    struct state_regs regs;
    uint32_t tid;
    uint8_t priority;
    char name[MAX_TNAME_LENGTH + 1];
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
int thread_create(uint8_t priority, char *name, struct thread *thread, thread_function func, void *aux);
void thread_block(struct thread *thread);
void thread_unblock(struct thread *thread);
void thread_exit();

#endif