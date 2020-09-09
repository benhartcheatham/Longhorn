#ifndef _PROCESS_H
#define _PROCESS_H

/* includes */
#include <stddef.h>
#include <stdint.h>
#include "thread.h"
#include "../libk/list.h"
#include "../libk/stream.h"

/* defines */
#define MAX_NUM_THREADS 8
#define MAX_NAME_LENGTH 12

/* structs */
enum proc_states {PROCESS_READY, PROCESS_BLOCKED, PROCESS_DYING, PROCESS_TERMINATED};

struct process {
    uint32_t pid;
    enum proc_states state;
    char name[MAX_NAME_LENGTH + 1];
    struct thread *active_thread;
    struct thread threads[MAX_NUM_THREADS];
    uint8_t num_live_threads;

    //might want to make these FILE structs later on
    //shouldn't be accessed directly
    std_stream in, out, err;

    list_node_t node;
};

/* typedefs */
typedef void (proc_function) (void *aux);

/* functions */

/* initialization functions */
void init_processes();

/* process state functions */
int proc_create(char *name, proc_function , void *aux);
int proc_exit(struct process *proc);
int proc_kill(struct process *proc);
void proc_block(struct process *proc);
void proc_unblock(struct process *proc);

/* process "setter" functions */
void proc_set_running();

/* process "getter" functions */
struct process *proc_get_running();
uint8_t get_live_t_count(struct process *proc);

/* process stream functions */
char *get_in(struct process *proc);
char *get_out(struct process *proc);
char *get_err(struct process *proc);
void flush_in(struct process *proc);
void flush_out(struct process *proc);
void flush_err(struct process *proc);
int append_in(struct process *proc, char c);
int append_out(struct process *proc, char c);
int append_err(struct process *proc, char c);
//don't include a shrink_err right now because i want other processes to not be able to only erase certain parts of the stream
int shrink_in(struct process *proc, uint32_t size);
int shrink_out(struct process *proc, uint32_t size);

#endif