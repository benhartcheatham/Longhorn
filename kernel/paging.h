/* Defines functionality for paging. The paging system is layed out as follows:
 * 
 *  Virtual address space layout:                   Physical address space layout:
 *  +--------------------------+ 0xFFFFFFFF         +--------------------------+
 *  | KERNEL VIRUTAL ADDRESSES |                    |           UNUSED         |
 *  +--------------------------+ KVADDR_OFFSET      +--------------------------+ 0x?
 *  | KERNEL LOGCIAL ADDRESSES |                    |                          |
 *  +--------------------------+ KADDR_OFFSET       |                          |
 *  |                          |                    |                          |
 *  |                          |                    |                          |
 *  |                          |                    |       FREE MEMORY        | *** See physical address space section
 *  |       USER SPACE         |                    |                          | below for important info on this section ***
 *  |                          |                    |                          |
 *  |                          |                    |                          |
 *  |                          |                    |                          |
 *  |                          |                    +--------------------------+ 0x? (_kernel_end)
 *  |                          |                    |          KERNEL          |
 *  +--------------------------+ 0x1000             +--------------------------+ 0x100000 (1MB)
 *  |   NULL PAGE (UNMAPPED)   |                    |        BIOS MEMORY       |
 *  +--------------------------+ 0x0                +--------------------------+ 0x0
 * 
 * In the virtual address space, the NULL page is unmapped for both the kernel and user, and is an error to access.
 * Kernel logical addresses map to the physcial address space by an offset (in other words, a kernel logical address
 * is equivalent to physical address + KADDR_OFFSET). Kernel virtual addresses are initially unmapped and mapped in
 * as the kernel needs them. User space is where all user accessible pages live, and are accessible by the kernel as well.
 * 
 * In the physical address space, the kernel is loaded at 1MB (accessible in the extern variable _kernel_start) and ends
 * at the location in the extern vairable _kernel_end. The unused region at the top of the address space is marked as
 * unusable memory by the BIOS, and is therefore unused. It is also worth noting that the free memory section of
 * the address space also has some holes in it due to the BIOS marking them as unusable, but they aren't shown in the
 * diagram above. Finally, free memory is the section of the address space used by the OS and user programs.
 */

#ifndef PAGING_H
#define PAGING_H

/* includes */
#include <stdint.h>
#include <stdbool.h>

/* defines */
#define KERNEL_TABLE  0
#define KADDR_OFFSET 0xC0000000
#define KVADDR_OFFSET 0xF0000000
#define PG_ROUND_UP(x) ((((uint32_t) x) + PG_SIZE + 1) & (~(PG_SIZE - 1)))
#define PG_ROUND_DOWN(x) (((uint32_t) x) & (~(PG_SIZE - 1)))

/* typedefs */

// these typedefs are for use elsewhere in the OS and in paging.c.
typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;
typedef uint32_t kaddr_t;
typedef uint32_t kvaddr_t;

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

// a pde is the same thing as a page_table struct *
typedef struct pde {
    uint32_t present  : 1;
    uint32_t rw       : 1;
    uint32_t user     : 1;
    uint32_t pwt      : 1;
    uint32_t cache    : 1;
    uint32_t accessed : 1;
    uint32_t ignored  : 6;  // must be zeroed out because bit 7 needs to be zero
    uint32_t table_addr     : 20;
} pde_t;

typedef struct page_table {
    struct pte entries[1024];
} page_table_t __attribute__ ((aligned(4096)));

typedef struct page_dir {
    struct pde tables[1024];
} page_dir_t __attribute__ ((aligned(4096)));


/* functions */
int init_paging();
int paging_init(page_dir_t *proc);
int paging_map(page_dir_t *pg_dir, vaddr_t vaddr, paddr_t paddr);
int paging_kmap(page_dir_t *pg_dir, paddr_t paddr);
int paging_kvmap(page_dir_t *pg_dir, kvaddr_t kvaddr, paddr_t paddr);
page_dir_t *get_current_pgdir_phys();
page_dir_t *get_current_pgdir();
page_table_t *paging_traverse_pgdir(page_dir_t *pg_dir, vaddr_t vaddr, bool create);
pte_t *paging_traverse_pgtable(page_table_t *pg_table, vaddr_t vaddr);
#endif