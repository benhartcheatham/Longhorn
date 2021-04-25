/* Implements the process subsystem of the kernel. Processes are not a schedulable unit, but are instead a wrapper/container
 * for threads, which are the default scheduable unit. */

/* includes */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kerrors.h>
#include <list.h>
#include <mem.h>
#include "../drivers/vesa.h"
#include "proc.h"
#include "palloc.h"
#include "paging.h"

/* defines */

/* globals */
static struct list all_procs;
static struct process *current;
static struct process *active;
static uint32_t pid_count;

/* prototypes */
static int proc_get_free_thread(struct process *proc);

/* functions */

/* initialization functions */

/** initializes the process subsystem */
void init_processes(struct process *init) {
    list_init(&all_procs);

    //create init process
    if (init == NULL) {
        char *stop = NULL;
        *stop = 0;
        asm volatile("cli");
        asm volatile("hlt");
    }

    struct process *p = init;

    init_std(&p->std_in);
    init_std(&p->std_out);
    init_std(&p->std_err);
    flush_std(&p->std_in);
    flush_std(&p->std_out);
    flush_std(&p->std_err);

    p->stdin = &p->std_in;
    p->stdout = &p->std_out;
    p->stderr = &p->std_err;

    sprintf(p->name, "init");
    pid_count = 0;
    p->pid = pid_count++;

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++)
        p->threads[i] = NULL;

    current = p;

    list_init(&p->waiters);
    p->wait_code = 0;

    init_threads(p);
    p->active_thread = p->threads[0];

    list_insert(&all_procs, &p->node);
}

/* process state functions */

/** creates a process
 * 
 * @param name: name of the process
 * @param func: function for the main thread of the process to execute
 * @param aux: parameters for func and any other data
 * 
 * @return -PROC_CREATE_FAIL on failure, pid of proc otherwise
 */
struct process *proc_create(char *name, proc_function func, void *aux) {
    //change this to just use kmalloc
    struct process *p = (struct process *) palloc();
    if (p == NULL)
        return NULL;
    
    sprintf(p->name, "%s", name);

    p->pid = pid_count++;

    init_std(&p->std_in);
    init_std(&p->std_out);
    init_std(&p->std_err);
    flush_std(&p->std_in);
    flush_std(&p->std_out);
    flush_std(&p->std_err);

    p->stdin = &p->std_in;
    p->stdout = &p->std_out;
    p->stderr = &p->std_err;

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        p->threads[i] = NULL;
    }


    list_init(&p->waiters);
    p->wait_code = 0;

    p->magic = PROC_MAGIC;

    if (thread_create(0, "main", p, 0, func, aux) != -THREAD_CREATE_FAIL)
        p->num_live_threads = 1;
    else {
        return NULL;
    }

    p->active_thread = p->threads[0];
    list_insert_end(&all_procs.tail, &p->node);

    return p;
}

int proc_create_thread(uint8_t priority, char *name, thread_function func, void *aux) {
    int thread_slot = proc_get_free_thread(PROC_CUR());
    if (thread_slot == -1)
        return -1;
    
    int tid = thread_create(priority, name, PROC_CUR(), thread_slot, func, aux);

    if (tid != -THREAD_CREATE_FAIL)
        PROC_CUR()->num_live_threads++;
    
    return tid;
}

/** exit the current process
 * 
 * @param ret: return code of the process
 */
void proc_exit(int *ret) {
    proc_kill(PROC_CUR(), ret);
}

/** notifes either the first thread waiting on this process
 * or all threads waiting on this process to wake
 * 
 * @param all: whether to notify the first or all threads
 * @param ret: code to return to waiting thread(s)
 * 
 * @return -1 on failure, 0 on success
 */
int proc_notify(struct process *p, bool all, int ret) {
    list_t *waiters = &p->waiters;

    list_node_t *node = list_pop(waiters);
    if (node == NULL)
        return -1;

    struct thread *wait_t;
    do {
        wait_t = LIST_ENTRY(node, struct thread, wait_node);
        wait_t->wait_code = ret;
        thread_unblock(wait_t);
        node = list_pop(waiters);

    } while (all && node != NULL);

    return 0;
}

/** causes this thread to wait on a process p
 * this call blocks until p notifies this thread or
 * until p dies
 * 
 * @param p: process to wait on
 * 
 * @return -1 if wait fails, return of thread once wait is complete
 */
int proc_wait(struct process *p) {
    if (p == NULL)
        return -1;

    list_insert(&p->waiters, &THREAD_CUR()->wait_node);
    thread_block();

    return p->wait_code;
}

/** cleans up any book keeping for process p
 * the actual resources should be deallocated in thread_kill
 * 
 * @param p: process to clean up
 */
void proc_cleanup(struct process *p) {
    if (p == NULL || p->num_live_threads != 0) 
        return;

    proc_notify(p, true, 0);
    list_delete(&all_procs, &p->node);
}

/** kill a process
 * 
 * @param proc: process to kill
 * @param ret: pointer to where to store retun code, if non-NULL
 */
void proc_kill(struct process *proc, int *ret) {
    if (proc == NULL)
        return;

    int num_alive = proc->num_live_threads;
    for (int i = 0; i < MAX_NUM_THREADS; i++) {
        if (proc->threads[i] != NULL && proc->threads[i]->state != THREAD_DYING 
                                    && proc->threads[i]->state != THREAD_RUNNING) {
            int t_ret = thread_kill(proc->threads[i]);

            if (t_ret != -1)
                num_alive--;
        }
    }

    if (ret != NULL)
        *ret = num_alive;

    if (PROC_CUR() == proc && num_alive == 1) {
        if (ret != NULL)
            *ret = --num_alive;

        thread_exit(NULL);
    }
}

/* process "setter" functions */

/** sets the active thread of the process
 * the active thread of a process is the thread currently executing
 * 
 * @param proc: process to set active thread of
 * @param num: which thread to set as active in process thread table
 */
void proc_set_active_thread(struct process *proc, uint8_t num) {
    proc->active_thread = proc->threads[num];
}

/** sets the active process to proc with pid
 * 
 * @param pid: pid of process to set as active
 */
void proc_set_active(uint32_t pid) {
    list_node_t *node = all_procs.head.next;
    struct process *proc = LIST_ENTRY(node, struct process, node);

    while (list_hasNext(node)) {
        if (proc != NULL && proc->pid == pid) {
            active = proc;
            break;
        }

        node = node->next;
        proc = LIST_ENTRY(node, struct process, node);
    }
}

/** sets the active process to proc
 * 
 * @param proc: process to set as the active process
 */
void proc_set_active_p(struct process *proc) {
    active = proc;
}

/* process "getter" functions */

/** gets the state of the active thread of the process
 * 
 * @param p: process to get the state from
 */
enum thread_states proc_get_state(struct process *p) {
    return p->active_thread->state;
}

/** returns a pointer to the active process 
 * 
 * @return pointer to the active process
 */
struct process *proc_get_active() {
    return active;
}

/** returns a pointer to the all list for processes
 * don't use this unless necessary, shouldn't modify this list
 * 
 * @return head of the process all list
 */
const list_node_t *proc_peek_all_list() {
    return list_peek(&all_procs);
}

/** gets the amount of live threads process proc owns 
 * a live thread is a proces that has a state other than TERMINATED
 * or has yet to be created
 * 
 * @param proc: process to get number of live threads from
 * 
 * @return number of live threads of proc
 */
uint8_t proc_get_live_t_count(struct process *proc) {
    return proc->num_live_threads;
}

/* static functions */

/** gets the next free slot in the threads array if there is one 
 * 
 * @param proc: process to get the next free slot from
 * 
 * @return pointer to next free thread in the process
 */
static int proc_get_free_thread(struct process *proc) {
    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++)
        if (proc->threads[i] == NULL)
            return i;
    return -1;
}





