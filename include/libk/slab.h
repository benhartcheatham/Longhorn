/* Defines the slab allocator interface and some extra functionality */
#ifndef SLAB_H
#define SLAB_H

/* includes */
#include <stddef.h>

/* defines */

/* structs */
struct slab_allocator {
    /** allocates memory in the slab allocator
     * 
     * @param s: slab allocator to allocate from
     * @param num_slabs: number of slabs to allocate
     * 
     * @return address of allocated memory
     */
    void *(*alloc)(struct slab_allocator *s, size_t num_slabs);

    /** frees memory in the slab allocator
     * 
     * @param s: slab allocator to free memory in
     * @param addr: address of prior allocation to free
     * @param num_slabs: size of prior allocation to free
     * 
     * @return implementation dependent
     */
    int (*free)(struct slab_allocator *s, void *addr, size_t num_slabs);

    /** function to initialize a slab allocator
     * 
     * @param s: slab allocator to initialize
     * @param mem: pointer to memory to be used by slab allocator
     * @param mem_size: size of memory given to slab allocator to use
     * @param slab_size: size of each slab in the allocator
     * @param aux: any extra information to be used by the implementation
     * 
     * @return implementation dependent
     */
    int (*init)(struct slab_allocator *s, void *mem, size_t mem_size, size_t slab_size, void *aux);

    /* pointer to the memory region */
    void *mem;
    /* size of the memory region */
    size_t mem_size;
    /* size of the free memory in the memory region */
    size_t free_mem_size;
    /* size of the slabs to allocate */
    size_t slab_size;
    /* pointer to the start of the underlying bookkeeping struct */
    void *data;
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