/* Declares functions to access the kernel page frame allocator. */

#ifndef PALLOC_H
#define PALLOC_H

/* includes */
#include <stddef.h>
#include "../boot/multiboot.h"

/* defines */
#define PG_SIZE 4096
#define KB 1024
#define MB 1024*KB
#define ROUND_UP(x, size) (((x + size - 1) / size) * size)

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

/* testing and status functions */
#ifdef TESTS
size_t num_allocated();
size_t map_size();
#endif

#endif