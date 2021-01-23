#ifndef _ALLOC_H
#define _ALLOC_H

/* includes */
#include <stddef.h>
#include "../boot/multiboot.h"

/* defines */
#define PG_SIZE 4096
#define KB 1024
#define MB 1024*KB

/* structs */

/* typedefs */

/* functions */

/* initialization functions */
void init_alloc(multiboot_info_t *mb);

/* page allocation functions */
void *palloc();
void *palloc_mult(size_t cnt);
int pfree(void *addr);
int pfree_mult(void *addr, size_t cnt);

/* variable size allocation functions */
void *kmalloc(size_t size);
void *kcalloc(size_t num, size_t size);
int kfree(void *addr);

/* testing and status functions */
size_t num_allocated();
size_t map_size();

#endif