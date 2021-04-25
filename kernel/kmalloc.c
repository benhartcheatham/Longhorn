/* This file is used for smaller allocations through malloc, if you need allocations
 * of a page size or bigger, consider using one of the palloc implementations. */

/* includes */
#include <stdint.h>
#include <stdbool.h>
#include <kerrors.h>
#include <synch.h>
#include <list.h>
#include <slab.h>
#include "kmalloc.h"
#include "palloc.h"

/* defines */

/* structs */
struct allocation {
    void *addr;
    size_t size;

    struct list_node node;
};

/* globals */
static slab_alloc_t *allocator;
static spin_lock_t malloc_lock;
static list_t allocations;

/* functions */

/** initalizes kmalloc
 * 
 * @param addr: address to start kmalloc heap at
 * @param size: size of kmalloc heap
 * @param slab_size: size of each slab in kmalloc heap
 * 
 * @return -1 on failure, 0 on success
 */
int init_kmalloc(void *addr, size_t size, size_t slab_size) {
    if (slab_size < sizeof(struct allocation))
        return -1;
    
    list_init(&allocations);
    slab_init(get_default_slab_allocator(), addr, size, slab_size, NULL);
    allocator = get_default_slab_allocator();
    spin_lock_init(&malloc_lock);

    return 0;
}

/** obtains the memory address of a memory area of at least size and 
 * no greater than PG_SIZE from the memory manager
 * 
 * @param size: requested size in bytes of allocation
 * 
 * @return address of allocated memory, NULL if no such region exists
 */
void *kmalloc(size_t size) {
    void *ret = NULL;

    size_t num_slabs = num_slabs = ROUND_UP(size / allocator->slab_size, allocator->slab_size);
    if (num_slabs == 0)
        num_slabs++;
    
    size_t num_alloc_slabs = ROUND_UP(sizeof(struct allocation) / allocator->slab_size, allocator->slab_size);
    if (num_alloc_slabs == 0)
        num_alloc_slabs++;

    if (spin_lock_acquire(&malloc_lock) != LOCK_ACQ_SUCC)
        return NULL;
    
    // don't even attempt allocations that are bigger than the free mem left
    if (ROUND_UP(size, allocator->slab_size) > allocator->free_mem_size) {
        spin_lock_release(&malloc_lock);
        return palloc();
    }
    
    // allocate memory requested
    ret = allocator->alloc(allocator, num_slabs);
    if (ret == NULL) {
        spin_lock_release(&malloc_lock);
        return NULL;
    }

    struct allocation *a = allocator->alloc(allocator, num_alloc_slabs);
    if (a == NULL) {
        allocator->free(allocator, ret, num_slabs);
        spin_lock_release(&malloc_lock);
        return NULL;
    }

    a->addr = ret;
    a->size = num_slabs;
    list_insert(&allocations, &a->node);

    spin_lock_release(&malloc_lock);
    return ret;
}

/** obtains the memory address of a zeroed memory area of at least num * size 
 * from the memory manager
 *  * 
 * @param num: number of allocations
 * @param size: size of allocations
 * 
 * @return address of allocated memory, NULL if no such region exists
 */
void *kcalloc(size_t num, size_t size) {
    uint64_t *mem = (uint64_t *) kmalloc(num * size);
    if (mem == NULL)
        return NULL;
    
    uint64_t i;
    // zero out the whole memory region kmalloc gives us
    for (i = 0; i < ROUND_UP(num * size / allocator->slab_size, allocator->slab_size); i++)
        mem[i] = 0;
    
    return (void *) mem;
}

/** frees a unit of memory gotten from kmalloc or kcalloc
 * 
 * @param addr: address of previous memory allocation
 * 
 * @return -MEM_FREE_FAIL on failure, MEM_FREE_SUCC otherwise
 */
int kfree(void *addr) {
    if (spin_lock_acquire(&malloc_lock) != LOCK_ACQ_SUCC)
        return -MEM_FREE_FAIL;
    
    slab_alloc_t *allocator = get_default_slab_allocator();
    struct list_node *node = &allocations.head;

    // find previous allocation, if it exists
    while(list_hasNext(node)) {
        struct allocation *a = LIST_ENTRY(node, struct allocation, addr);

        if (a->addr == addr) {
            int ret = MEM_FREE_SUCC;
            // free previous allocation
            if (allocator->free(allocator, addr, a->size) != SLAB_SUCC)
                ret = -MEM_FREE_FAIL;
            
            spin_lock_release(&malloc_lock);
            return ret;
        }

        node = node->next;
    }

    // allocation wasn't found
    spin_lock_release(&malloc_lock);
    return pfree(addr);
}