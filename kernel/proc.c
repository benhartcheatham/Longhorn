#include <stddef.h>
#include <stdint.h>
#include "proc.h"
#include "kalloc.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#include "../libc/stdio.h"
#include "../libk/list.h"

/* static data */
static struct list all_procs;
static struct process *current;
static struct process *active;
static uint32_t pid_count;

/* static functions */
static struct thread *proc_get_free_thread(struct process *proc);

/* initialization functions */
void init_processes() {
    list_init(&all_procs);

    //create init process
    struct process *p = (struct process *) palloc();
    if (p == NULL) {
        println("CRITICAL ERROR: NO MEMORY FOR INIT PROCESS.\nHALTING...");
        asm volatile("cli");
        asm volatile("hlt");
    }

    init_std(&p->stdin);
    init_std(&p->stdout);
    init_std(&p->stderr);
    flush_std(&p->stdin);
    flush_std(&p->stdout);
    flush_std(&p->stderr);

    sprintf(p->name, "init");
    p->node._struct = (void *) p;
    pid_count = 0;
    p->pid = pid_count++;
    p->state = PROCESS_RUNNING;

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        p->threads[i].state = THREAD_TERMINATED;
        p->threads[i].child_num = i;
    }

    current = p;
    
    //create init thread
    struct thread *init_t = &p->threads[0];
    init_t->parent = &p->node;
    init_t->state = THREAD_RUNNING;
    init_t->tid = 0;
    char *init_tname = "init:init_t";
    memcpy(init_t->name, init_tname, strlen(init_tname) + 1);
    asm volatile("mov %%esp, %0" : "=g" (init_t->esp));
    init_t->esp = (uint32_t *) ((uint32_t) init_t->esp);
    init_threads(p);

    p->active_thread = &p->threads[0];
    list_insert(&all_procs, &p->node);
}

/* process state functions */

int proc_create(char *name, proc_function func, void *aux) {
    //change this to just use kmalloc
    struct process *p = (struct process *) palloc();
    if (p == NULL)
        return -1;
    
    sprintf(p->name, "%s", name);
    p->node._struct = (void *) p;

    p->pid = pid_count++;
    p->state = PROCESS_READY;

    init_std(&p->stdin);
    init_std(&p->stdout);
    init_std(&p->stderr);
    flush_std(&p->stdin);
    flush_std(&p->stdout);
    flush_std(&p->stderr);

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        p->threads[i].state = THREAD_TERMINATED;
        p->threads[i].child_num = i;
    }

    if (thread_create(0, "main_t", &p->node, &p->threads[0], func, aux) > -1)
        p->num_live_threads = 1;
    else {
        proc_kill(p);
        return -1;
    }
    
    p->active_thread = &p->threads[0];
    list_insert_end(&all_procs.tail, &p->node);
    return p->pid;
}

int proc_create_thread(uint8_t priority, char *name, thread_function func, void *aux) {
    struct thread *t = proc_get_free_thread(proc_get_running());
    int tid = thread_create(priority, name, &proc_get_running()->node, t, func, aux);

    if (tid > -1)
        proc_get_running()->num_live_threads++;
    
    return tid;
}

/* intended for a graceful exit */
int proc_exit(struct process *proc) {
    if (proc_kill(proc) < 0)
        return -1;
    
    return 0;
}

int proc_kill(struct process *proc) {
    int i;
    for (i = 0; i < MAX_NUM_THREADS && proc->num_live_threads != 0; i++) {
        int kill_return = thread_kill(&proc->threads[i]);
        if (kill_return != (int) proc->threads[i].tid) {
            printf("COULDN'T KILL THREAD: %s WITH TID: %d\n", proc->threads[i].name, proc->threads[i].tid);
            printf("THREAD TID: %d PROC->THREAD TID: %d\n", kill_return, proc->threads[i].tid);
            return -1;
        }
        
        proc->num_live_threads--;
        if (proc->num_live_threads <= 0)
            break; 
        
    }

    proc->state = PROCESS_DYING;
    list_delete(&all_procs, &proc->node);
    int proc_pid = (int) proc->pid;
    pfree(proc);

    return proc_pid;
}

void proc_block(struct process *proc) {
    proc->state = PROCESS_BLOCKED;
}

void proc_unblocked(struct process *proc) {
    proc->state = PROCESS_READY;
}

/* process "setter" functions */

/* sets the current process to the parent of the current thread */
void proc_set_running() {
    current->state = PROCESS_READY;

    current = (struct process *) thread_get_running()->parent->_struct;
    current->active_thread = thread_get_running();
    current->state = PROCESS_RUNNING;
}

/* sets the active process to proc */
void proc_set_active(uint32_t pid) {
    list_node *node = all_procs.head.next;
    struct process *proc = (struct process *) node->_struct;

    while (list_hasNext(node)) {
        if (proc != NULL && proc->pid == pid) {
            active = proc;
            break;
        }

        node = node->next;
        proc = (struct process *) node->_struct;
    }
}

/* process "getter" functions */

/* returns a pointer to the current process */
struct process *proc_get_running() {
    return current;
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
        if (proc->threads[i].state == THREAD_TERMINATED)
            return &proc->threads[i];
    return NULL;
}





