/* Defines functionality for paging. */

#ifndef PAGING_H
#define PAGING_H

/* includes */
#include <stdint.h>
#include "proc.h"

/* defines */
#define KERNEL_TABLE  0
#define KADDR_OFFSET 0xC0000000

/* typedefs */

// these typedefs are for use elsewhere in the OS and in paging.c.
typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;
typedef uint32_t kaddr_t;

/* structs */
typedef struct pte {
    uint32_t present  : 1;
    uint32_t rw       : 1;
    uint32_t user     : 1;
    uint32_t pwt      : 1;
    uint32_t cache    : 1;
    uint32_t accessed : 1;
    uint32_t dirty    : 1;
    uint32_t pat      : 1;
    uint32_t global   : 1;
    uint32_t ignored  : 3;
    uint32_t addr     : 20;
} pte_t;

// a pde is the same thing as a page_table struct
typedef struct pde {
    uint32_t present  : 1;
    uint32_t rw       : 1;
    uint32_t user     : 1;
    uint32_t pwt      : 1;
    uint32_t cache    : 1;
    uint32_t accessed : 1;
    uint32_t ignored  : 6;  // must be zeroed out because bit 7 needs to be zero
    uint32_t addr     : 20;
} pde_t;

typedef struct page_table {
    struct pte entries[1024];
} page_table_t __attribute__ ((aligned(4096)));

typedef struct page_dir {
    struct pde tables[1024];
} page_dir_t __attribute__ ((aligned(4096)));


/* functions */
int init_paging(page_dir_t *pd);
int paging_init(struct process *proc, struct process *proc_parent);
int paging_map(vaddr_t vaddr, paddr_t paddr);
int paging_kmap(kaddr_t kaddr, paddr_t paddr);


#endif