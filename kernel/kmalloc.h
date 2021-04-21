
#include <stddef.h>


int init_kmalloc(void *addr, size_t size, size_t slab_size);
void *kmalloc(size_t size);
void *kcalloc(size_t num, size_t size);
int kfree(void *addr);