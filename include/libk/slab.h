#ifndef SLAB_H
#define SLAB_H

/* includes */
#include <stddef.h>

/* structs */
struct slab_allocator {
    void *(*alloc)(struct slab_allocator *s, size_t num_slabs);
    int (*free)(struct slab_allocator *s, void *addr, size_t num_slabs);
    int (*init)(struct slab_allocator *s, void *mem, size_t mem_size, size_t slab_size, void *aux);

    void *mem;  // pointer to the memory region
    size_t mem_size;    // size of the memory region
    size_t free_mem_size;   // not used internally, just for debugging later
    size_t slab_size;   // size of the slabs to allocate

    void *data;    // pointer to the start of the underlying bookkeeping struct
};

/* typedefs */
typedef struct slab_allocator slab_alloc_t;
typedef void *(*slab_alloc_func)(slab_alloc_t *s, size_t num_slabs);
typedef int (*slab_free_func)(slab_alloc_t *s, void *addr, size_t num_slabs);
typedef int (*slab_init_func)(slab_alloc_t *s, void *mem, size_t mem_size, size_t slab_size, void *aux);

/* functions */
slab_alloc_t *get_default_slab_allocator();
int slab_init(slab_alloc_t *s, void *mem, size_t mem_size, size_t slab_size, void *aux);
#ifdef TESTS
    void slab_print_list(slab_alloc_t *s);
#endif

#endif