#ifndef _THREAD_H
#define _THREAD_H

/* includes */
#include <stdint.h>
#include "proc.h"
#include "../libk/list.h"

/* defines */
#define MAX_TNAME_LENGTH 25
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
    char name[MAX_TNAME_LENGTH];
    enum thread_states state;

    struct process *parent;
    uint32_t child_num;

    struct list_node node;
};

/* typedefs */
typedef void (thread_function) (void *aux);
/* functions */

/* initialization functions */
void init_threads(struct process *init);

/* thread state functions */
int thread_create(uint32_t priority, char *name, thread_function func, void *aux);
void thread_block(struct thread *thread);
void thread_unblock(struct thread *thread);
void thread_exit();

#endif