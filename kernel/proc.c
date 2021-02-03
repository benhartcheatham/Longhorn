#include <stddef.h>
#include <stdint.h>
#include "proc.h"
#include "kalloc.h"
#include <mem.h>
#include <string.h>
#include <stdio.h>
#include <list.h>
#include "../drivers/vesa.h"

/* static data */
static struct list all_procs;
static struct process *current;
static struct process *active;
static uint32_t pid_count;

struct process init;

/* static functions */
static struct thread *proc_get_free_thread(struct process *proc);

/* initialization functions */
void init_processes() {
    list_init(&all_procs);

    //create init process
    struct process *init = (struct process *) palloc();
    if (init == NULL) {
        println("CRITICAL ERROR: NO MEMORY FOR INIT PROCESS.\nHALTING...");
        asm volatile("cli");
        asm volatile("hlt");
    }

    init_std(&init->stdin);
    init_std(&init->stdout);
    init_std(&init->stderr);
    flush_std(&init->stdin);
    flush_std(&init->stdout);
    flush_std(&init->stderr);

    sprintf(init->name, "init");
    pid_count = 0;
    init->pid = pid_count++;

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        init->threads[i]->state = THREAD_TERMINATED;
        init->threads[i]->child_num = i;
    }

    current = init;
    
    init_threads(init);
    init->active_thread = init->threads[0];

    list_insert(&all_procs, &init->node);
}

/* process state functions */

int proc_create(char *name, proc_function func, void *aux) {
    //change this to just use kmalloc
    struct process *p = (struct process *) palloc();
    if (p == NULL)
        return -1;
    
    sprintf(p->name, "%s", name);

    p->pid = pid_count++;

    init_std(&p->stdin);
    init_std(&p->stdout);
    init_std(&p->stderr);
    flush_std(&p->stdin);
    flush_std(&p->stdout);
    flush_std(&p->stderr);

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        p->threads[i]->state = THREAD_TERMINATED;
        p->threads[i]->child_num = i;
    }

    if (thread_create(0, "main", p, &p->threads[0], func, aux) > -1)
        p->num_live_threads = 1;
    else {
        proc_kill(p, NULL);
        return -1;
    }
    
    p->active_thread = p->threads[0];

    list_insert_end(&all_procs.tail, &p->node);
    return p->pid;
}

int proc_create_thread(uint8_t priority, char *name, thread_function func, void *aux) {
    struct thread *t = proc_get_free_thread(PROC_CUR());
    int tid = thread_create(priority, name, PROC_CUR(), &t, func, aux);

    if (tid > -1)
        PROC_CUR()->num_live_threads++;
    
    return tid;
}

/* intended for a graceful exit */
void proc_exit(int *ret) {
    proc_kill(PROC_CUR(), ret);
}

void proc_kill(struct process *proc, int *ret) {
    int success = 0;

    int i;
    for (i = 0; i < MAX_NUM_THREADS && proc->num_live_threads > 1; i++) {
        if (proc->threads[i]->tid != THREAD_CUR()->tid) {
            int kill_return = thread_kill(proc->threads[i]);
            if (kill_return != 0) {
                printf("COULDN'T KILL THREAD: %s WITH TID: %d\n", proc->threads[i]->name, proc->threads[i]->tid);
                printf("THREAD TID: %d PROC->THREAD TID: %d\n", kill_return, proc->threads[i]->tid);
                success = -1;
            }
            
            if (proc->num_live_threads <= 0)
                break; 
        }
    }

    if (success == -1 && ret != NULL) {
        *ret = -1;
        return;
    } else if (ret != NULL)
        *ret = 0;

    list_delete(&all_procs, &proc->node);
    pfree(proc);
    
    THREAD_CUR()->state = THREAD_DYING;
    thread_yield();
}

/* process "setter" functions */

void proc_set_active_thread(struct process *proc, uint8_t num) {
    proc->active_thread = proc->threads[num];
}

/* sets the active process to proc with pid*/
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

/* sets the active process to proc */
void proc_set_active_p(struct process *proc) {
    active = proc;
}

/* process "getter" functions */

/* gets the state of the active thread of the process */
enum thread_states proc_get_state(struct process *p) {
    return p->active_thread->state;
}

/* returns a pointer to the active process */
struct process *proc_get_active() {
    return active;
}

/* returns a pointer to the all list for processes
   don't use this unless necessary, should be using a list
   iterator that doesn't allow for modification */
const list_node *proc_peek_all_list() {
    return list_peek(&all_procs);
}

/* gets the amount of live threads process proc owns */
uint8_t proc_get_live_t_count(struct process *proc) {
    return proc->num_live_threads;
}

/* static functions */

/* gets the next free slot in the threads array if there is one */
static struct thread *proc_get_free_thread(struct process *proc) {
    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++)
        if (proc->threads[i] == NULL)
            return proc->threads[i];
    return NULL;
}





