/* file meant for a malloc implementation to be used in the kernel. */

#ifndef KMALLOC_H
#define KMALLOC_H

/* includes */
#include <stddef.h>

/* defines */

/* structs */

/* typedefs */

/* functions */
int init_kmalloc(void *addr, size_t size, size_t slab_size);
void *kmalloc(size_t size);
void *kcalloc(size_t num, size_t size);
int kfree(void *addr);

#endif