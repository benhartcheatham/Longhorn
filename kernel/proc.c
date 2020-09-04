#include <stddef.h>
#include <stdint.h>
#include "proc.h"
#include "../libc/mem.h"
#include "../libk/list.h"

static struct list ready_procs;
static struct list non_ready_procs;
uint32_t pid_count;

void init_processes() {

}

int proc_create(char *name, proc_function init_func) {
    return -1;
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

void append_in(struct process *proc, char c) {
    append_std(&proc->in, c);
}

void append_out(struct process *proc, char c) {
    append_std(&proc->out, c);
}

void append_err(struct process *proc, char c) {
    append_std(&proc->err, c);
}

void shrink_in(struct process *proc, uint32_t size) {
    shrink_std(&proc->in, size);
}

void shrink_out(struct process *proc, uint32_t size) {
    shrink_std(&proc->out, size);
}


