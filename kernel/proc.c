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
static uint32_t pid_count;

/* static functions */
static void set_name(struct process *proc, char *name);
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

    set_name(p, "init");
    p->node._struct = (void *) p;
    pid_count = 0;
    p->pid = pid_count++;
    p->state = PROCESS_READY;

    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++) {
        p->threads[i].state = THREAD_TERMINATED;
        p->threads[i].child_num = i;
    }

    current = p;
    
    //create init thread
    struct thread *init_t = &p->threads[0];
    init_t->parent = &p->node;
    init_t->state = THREAD_READY;
    init_t->tid = 0;
    char *init_tname = "init_t";
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
    
    set_name(p, name);
    p->node._struct = (void *) p;

    p->pid = pid_count++;
    p->state = PROCESS_READY;

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
    proc_kill(proc);
    return 0;
}

int proc_kill(struct process *proc) {
    int i;
    for (i = 0; i < MAX_NUM_THREADS && proc->num_live_threads != 0; i++) {
        if (thread_kill(&proc->threads[i]) != (int) proc->threads[i].tid)
            printf("COULDN'T KILL THREAD: %s WITH TID: %d\n", proc->threads[i].name, proc->threads[i].tid);
        
        proc->num_live_threads--;
        if (proc->num_live_threads <= 0)
            break; 
        
    }

    proc->state = PROCESS_DYING;
    list_delete(&all_procs, &proc->node);
    pfree(proc);

    return (int) proc_get_running()->pid;
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
    current = (struct process *) thread_get_running()->parent->_struct;
    current->active_thread = thread_get_running();
}

/* process "getter" functions */

/* returns a pointer to the current process */
struct process *proc_get_running() {
    return current;
}

/* gets the amount of live threads process proc owns */
uint8_t get_live_t_count(struct process *proc) {
    return proc->num_live_threads;
}

/* process stream functions */
char *get_in(struct process *proc) {
    return get_copy_std(&proc->in);
}

char *get_out(struct process *proc) {
    return get_copy_std(&proc->out);
}  

char *get_err(struct process *proc) {
    return get_copy_std(&proc->err);
}

void flush_in(struct process *proc) {
    flush_std(&proc->in);
}

void flush_out(struct process *proc) {
    flush_std(&proc->out);
}

void flush_err(struct process *proc) {
    flush_std(&proc->err);
}

/* tries to append c to the in stream of a proc, returns num chars appended*/
int append_in(struct process *proc, char c) {
    return append_std(&proc->in, c);
}

/* tries to append c to the out stream of a proc, returns num chars appended*/
int append_out(struct process *proc, char c) {
    return append_std(&proc->out, c);
}

/* tries to append c to the err stream of a proc, returns num chars appended*/
int append_err(struct process *proc, char c) {
    return append_std(&proc->err, c);
}

/* tries to delete size chars from a process' in stream, returns num chars deleted */
int shrink_in(struct process *proc, uint32_t size) {
    return shrink_std(&proc->in, size);
}

/* tries to delete size chars from a process' out stream, returns num chars deleted */
int shrink_out(struct process *proc, uint32_t size) {
    return shrink_std(&proc->out, size);
}

/* static functions */

/* sets the name of a process */
static void set_name(struct process *proc, char *name) {
    if (strlen(name) < MAX_NAME_LENGTH) {
        memcpy(proc->name, name, strlen(name));
        proc->name[strlen(name) + 1] = '\0';
    }
}

/* gets the next free slot in the threads array if there is one */
static struct thread *proc_get_free_thread(struct process *proc) {
    int i;
    for (i = 0; i < MAX_NUM_THREADS; i++)
        if (proc->threads[i].state == THREAD_TERMINATED)
            return &proc->threads[i];
    return NULL;
}



