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
#include "kalloc.h"

/* defines */

/* globals */
static struct list all_procs;
static struct process *current;
static struct process *active;
static uint32_t pid_count;

/* prototypes */
static struct thread *proc_get_free_thread(struct process *proc);

/* functions */

/* initialization functions */

/** initializes the process subsystem */
void init_processes() {
    list_init(&all_procs);

    //create init process
    struct process *p = (struct process *) palloc();
    if (p == NULL) {
        char *stop = NULL;
        *stop = 0;
        asm volatile("cli");
        asm volatile("hlt");
    }

    init_std(GET_STDIN(p));
    init_std(GET_STDOUT(p));
    init_std(GET_STDERR(p));
    flush_std(GET_STDIN(p));
    flush_std(GET_STDOUT(p));
    flush_std(GET_STDERR(p));

    sprintf(p->name, "init");
    pid_count = 0;
    p->pid = pid_count++;

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        p->threads[i]->state = THREAD_TERMINATED;
        p->threads[i]->child_num = i;
    }

    current = p;
    
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
int proc_create(char *name, proc_function func, void *aux) {
    //change this to just use kmalloc
    struct process *p = (struct process *) palloc();
    if (p == NULL)
        return -PROC_CREATE_FAIL;
    
    sprintf(p->name, "%s", name);

    p->pid = pid_count++;

    init_std(GET_STDIN(p));
    init_std(GET_STDOUT(p));
    init_std(GET_STDERR(p));
    flush_std(GET_STDIN(p));
    flush_std(GET_STDOUT(p));
    flush_std(GET_STDERR(p));

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        p->threads[i]->state = THREAD_TERMINATED;
        p->threads[i]->child_num = i;
    }

    if (thread_create(0, "main", p, &p->threads[0], func, aux) != -THREAD_CREATE_FAIL)
        p->num_live_threads = 1;
    else {
        proc_kill(p, NULL);
        return -PROC_CREATE_FAIL;
    }
    
    p->active_thread = p->threads[0];

    list_insert_end(&all_procs.tail, &p->node);
    return p->pid;
}

int proc_create_thread(uint8_t priority, char *name, thread_function func, void *aux) {
    struct thread *t = proc_get_free_thread(PROC_CUR());
    int tid = thread_create(priority, name, PROC_CUR(), &t, func, aux);

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

/** kill a process
 * 
 * @param proc: process to kill
 * @param ret: return code of the process
 */
void proc_kill(struct process *proc, int *ret) {
    int success = 0;

    int i;
    for (i = 0; i < MAX_NUM_THREADS && proc->num_live_threads > 1; i++) {
        if (proc->threads[i]->tid != THREAD_CUR()->tid) {
            int kill_return = thread_kill(proc->threads[i]);
            if (kill_return != THREAD_KILL_SUCC) {
                printf("COULDN'T KILL THREAD: %s WITH TID: %d\n", proc->threads[i]->name, proc->threads[i]->tid);
                printf("THREAD TID: %d PROC->THREAD TID: %d\n", kill_return, proc->threads[i]->tid);
                success = -THREAD_KILL_FAIL;
            }
            
            if (proc->num_live_threads <= 0)
                break; 
        }
    }

    if (success != THREAD_KILL_SUCC && ret != NULL) {
        *ret = -PROC_KILL_FAIL;
        return;
    } else if (ret != NULL)
        *ret = PROC_KILL_SUCC;

    list_delete(&all_procs, &proc->node);
    pfree(proc);
    
    THREAD_CUR()->state = THREAD_DYING;
    thread_yield();
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
    list_node *node = all_procs.head.next;
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
const list_node *proc_peek_all_list() {
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
static struct thread *proc_get_free_thread(struct process *proc) {
    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++)
        if (proc->threads[i] == NULL)
            return proc->threads[i];
    return NULL;
}





