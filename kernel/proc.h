#ifndef _PROCESS_H
#define _PROCESS_H

/* includes */
#include <stddef.h>
#include <stdint.h>
#include "../libk/list.h"
#include "../libk/stream.h"

/* defines */
#define MAX_NUM_THREADS 8
#define STD_STREAM_SIZE 256

/* structs */
enum proc_states {PROCESS_READY, PROCESS_BLOCKED, PROCESS_DYING, 
                            PROCESS_TERMINATED, PROCESS_SUSPENDED};

struct process {
    uint32_t pid;
    enum proc_states state;
    //struct thread *active_thread;
    //struct thread threads[MAX_NUM_THREADS];

    //might want to make these FILE structs later on
    //shouldn't be accessed directly
    std_stream in, out, err;
};

/* typedefs */
typedef void (*proc_function) (void *aux);

/* functions */

/* initialization functions */
void init_processes();

/* process state functions */
int proc_create(char *name, proc_function init_func);
void proc_kill_k(struct process *proc);
void proc_block(struct process *proc);
void proc_unblock(struct process *proc);


/* process stream functions */
char *get_in(struct process *proc);
char *get_out(struct process *proc);
char *get_err(struct process *proc);
void flush_in(struct process *proc);
void flush_out(struct process *proc);
void flush_err(struct process *proc);
void append_in(struct process *proc, char c);
void append_out(struct process *proc, char c);
void append_err(struct process *proc, char c);
//don't include a shrink_err right now because i want other processes to not be able to only erase certain parts of the stream
void shrink_in(struct process *proc, uint32_t size);
void shrink_out(struct process *proc, uint32_t size);
#endif