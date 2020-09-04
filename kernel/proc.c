#include <stddef.h>
#include <stdint.h>
#include "proc.h"
#include "kalloc.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#include "../libc/stdio.h"
#include "../libk/list.h"

/* static data */
static struct list ready_procs;
static struct list all_procs;
static uint32_t pid_count;

/* static functions */
static void set_name(struct process *proc, char *name);

/* initialization functions */
void init_processes() {
    list_init(&ready_procs);
    list_init(&all_procs);

    pid_count = 1;
}

/* process state functions */

int proc_create(char *name, proc_function init_func) {
    //change this to just use kmalloc
    struct process *p = (struct process *) palloc();
    set_name(p, name);
    p->node._struct = (void *) p;

    p->pid = pid_count++;
    p->state = PROCESS_READY;

    list_insert_end(&ready_procs.tail, &p->node);
    return p->pid;
}

void proc_kill_k(struct process *proc) {
    proc->state = PROCESS_DYING;
}

void proc_block(struct process *proc) {
    proc->state = PROCESS_BLOCKED;
}

void proc_unblocked(struct process *proc) {
    proc->state = PROCESS_READY;
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



