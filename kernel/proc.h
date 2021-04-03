/* Defines the process subsystem. */
#ifndef _PROCESS_H
#define _PROCESS_H

/* includes */
#include <stddef.h>
#include <stdint.h>
#include "thread.h"
#include <list.h>
#include <stream.h>

/* defines */
#define MAX_NUM_THREADS 8
#define MAX_NAME_LENGTH 12

/* structs */

struct process {

    uint32_t pid;   // unique identifier for the process
    char name[MAX_NAME_LENGTH + 1]; // name of the process
    struct thread *active_thread;   // the last thread to run, or currently running thread
    struct thread *threads[MAX_NUM_THREADS];    // the threads in this process, must be at least 1,
                                                // no more than MAX_NUM_THREADS
    uint8_t num_live_threads;   // nuber of alive threads in the process

    //might want to make these FILE structs later on
    //shouldn't be accessed directly
    std_stream stdin, stdout, stderr;   // std streams of the process

    list_node node; // node for all list of processes
};

/* typedefs */
typedef void (proc_function) (void *aux);

/* functions */

/* initialization functions */
void init_processes();

/* process state functions */
int proc_create(char *name, proc_function f, void *aux);
void proc_exit();
void proc_kill(struct process *proc, int *ret);

/* process "setter" functions */
void proc_set_active_thread(struct process *proc, uint8_t num);
void proc_set_active(uint32_t pid);
void proc_set_active_p(struct process *proc);

/* process "getter" functions */
enum thread_states proc_get_state(struct process *p);
struct process *proc_get_active();

const list_node *proc_peek_all_list();
uint8_t proc_get_live_t_count(struct process *proc);

/* gets the process that contains thread t */
inline struct process *get_thread_proc(struct thread *t) {
    return (struct process *) ((char *) t) + offsetof(thread_info_t, p);
}

#endif