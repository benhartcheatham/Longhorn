/* Defines the process subsystem. */
#ifndef _PROCESS_H
#define _PROCESS_H

/* includes */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stream.h>
#include <list.h>
#include "thread.h"


/* defines */
#define MAX_NUM_THREADS 8
#define MAX_NAME_LENGTH 12
#define PROC_MAGIC 0x34

/* structs */

struct process {

    uint32_t pid;   // unique identifier for the process
    char name[MAX_NAME_LENGTH + 1]; // name of the process
    struct thread *active_thread;   // the last thread to run, or currently running thread
    struct thread *threads[MAX_NUM_THREADS];    // the threads in this process, must be at least 1,
                                                // no more than MAX_NUM_THREADS
    uint8_t num_live_threads;   // nuber of alive threads in the process
    

    std_stream *stdin;  // stdin handle
    std_stream *stdout; // stdout handle 
    std_stream *stderr; // stderr handle
    std_stream std_in, std_out, std_err;   // std streams of the process

    list_t waiters; // list of waiting processes
    int wait_code;  // return code of process waited on
    list_node_t wait_node; // node to wait on processes with

    list_node_t node; // node for all list of processes
    uint32_t magic;
};

/* typedefs */
typedef void (proc_function) (void *aux);

/* functions */

/* initialization functions */
void init_processes();

/* process state functions */
struct process *proc_create(char *name, proc_function f, void *aux);
void proc_exit(int *ret);
void proc_kill(struct process *proc, int *ret);
int proc_wait(struct process *proc);
int proc_notify(struct process *proc, bool all, int ret);
void proc_cleanup(struct process *p);

/* process "setter" functions */
void proc_set_active_thread(struct process *proc, uint8_t num);
void proc_set_active(uint32_t pid);
void proc_set_active_p(struct process *proc);

/* process "getter" functions */
enum thread_states proc_get_state(struct process *p);
struct process *proc_get_active();

const list_node_t *proc_peek_all_list();
uint8_t proc_get_live_t_count(struct process *proc);

/** gets the process that contains thread t 
 * 
 * @param t: thread to get proc from
 * 
 * @return process that contains thread t
 */
inline struct process *get_thread_proc(struct thread *t) {
    thread_info_t *info = (thread_info_t *) t;
    return info->p;
}

#endif