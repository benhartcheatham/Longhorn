/* Defines functionality for paging. */

#ifndef PAGING_H
#define PAGING_H

/* includes */
#include <stdint.h>
/* defines */
#define KERNEL_TABLE  0
#define KVADDR_OFFSET 0xC0000000

/* structs */
struct pte {
    
} __attribute__ ((packed));

struct pde {
    
} __attribute__ ((packed));

struct page_table {
    struct pte *entries[1024];
} __attribute__ ((aligned(4096)));

struct page_dir {
    struct pde *tables[1024];
} __attribute__ ((aligned(4096)));

/* typedefs */

/* functions */


#endif