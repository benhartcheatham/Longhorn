

#include <stddef.h>
#include <mem.h>
#include "paging.h"
#include "palloc.h"

/* initializes paging system */
int init_paging(page_dir_t *pd) {
    return 0;
}

/* initalizes page tables for a process */
int paging_init(struct process *proc, struct process *proc_parent) {
    return 0;
}

/* maps a user virtual address to a physical address */
int paging_map(vaddr_t vaddr, paddr_t paddr) {
    return -1;
}

/* maps a kernel virtual address to a physical address */
int paging_kmap(kaddr_t kaddr, paddr_t paddr) {
    return 0;
}
