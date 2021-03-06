/* Implements the memory manager for the kernel. Currently supports two different allocators,
palloc and kmalloc. Palloc allocates from a bitmap in increments of PG_SIZE, while kmalloc
allocates from a slab allocator that allocates in increments of 64 bytes. */

/* includes */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <bitmap.h>
#include <kerrors.h>
#include <list.h>
#include <slab.h>
#include <synch.h>
#include "../boot/multiboot.h"
#include "kalloc.h"

/* defines */
#define ROUND_UP(x, size) (((x + size - 1) / size) * size)
#define SLAB_SIZE 64

/* structs */
struct allocation {
    void *addr;
    size_t size;

    struct list_node node;
};

/* globals */
static bitmap_t free_map;
static char *start_addr = (char *) (4*MB);
static spin_lock_t malloc_lock;
static spin_lock_t palloc_lock;
static list_t allocations;

/* functions */

/** initializes kmalloc and palloc 
 * 
 * @param mb: boot record given by GRUB2
 */
void init_alloc(multiboot_info_t *mb) {
    size_t num_pages = (size_t) (((mb->mem_upper * 1024) - 4*MB) / PG_SIZE);
    bitmap_init_s(&free_map, num_pages, start_addr);
    bitmap_set_range(&free_map, 0, num_pages / PG_SIZE + 1, true);

    list_init(&allocations);
    slab_init(get_default_slab_allocator(), palloc_mult(10), 10 * PG_SIZE, SLAB_SIZE, NULL);

    spin_lock_init(&malloc_lock);
    spin_lock_init(&palloc_lock);
}

/** gets the address of a free page of memory from the memory manager
 * 
 * @return address of allocated page, NULL if no page exists 
 */
void *palloc() {
    return palloc_mult(1);
}

/** gets the address of cnt consecutive free pages of memory from the memory manager
 * 
 * @param cnt: number of consecutive pages to allocate
 * 
 * @return address of allocated region, NULL if region doesn't exist
 */ 
void *palloc_mult(size_t cnt) {
    if (spin_lock_acquire(&palloc_lock) != LOCK_ACQ_SUCC)
        return NULL;
    
    size_t idx = bitmap_find_range(&free_map, 0, bitmap_get_size(&free_map), cnt, false);

    if (idx != bitmap_get_size(&free_map) + 1) {
        bitmap_set_range(&free_map, idx, cnt, true);
        spin_lock_release(&palloc_lock);
        return (void *) (start_addr + (idx * PG_SIZE));
    }
    
    spin_lock_release(&palloc_lock);
    return NULL;
}

/** frees a page of memory obtained from palloc
 * 
 * @param addr: address of previous allocation to free
 * 
 * @return -MEM_FREE_FAIL on failure, MEM_FREE_SUCC otherwise
 */
int pfree(void *addr) {
    return pfree_mult(addr, 1);
}

/** frees cnt pages of memory obtained from the memory manager
 * 
 * @param addr: address of previous allocation to free
 * @param cnt: size in pages of previous allocation
 * 
 * @return -MEM_FREE_FAIL on failure, MEM_FREE_SUCC otherwise
 */
int pfree_mult(void *addr, size_t cnt) {
    if (spin_lock_acquire(&palloc_lock) != LOCK_ACQ_SUCC)
        return -MEM_FREE_FAIL;
    
    size_t idx = ((char *) addr - start_addr) / PG_SIZE;

    if (idx + cnt < bitmap_get_size(&free_map) && bitmap_count_range(&free_map, idx, cnt) == cnt) {
        spin_lock_release(&palloc_lock);
        if (bitmap_set_range(&free_map, idx, cnt, false) == cnt)
            return MEM_FREE_SUCC;
        
        return -MEM_FREE_FAIL;
    }

    spin_lock_release(&palloc_lock);
    return -MEM_FREE_FAIL;
}

/** obtains the memory address of a memory area of at least size and 
 * no greater than PG_SIZE from the memory manager
 * 
 * @param size: requested size in bytes of allocation
 * 
 * @return address of allocated memory, NULL if no such region exists
 */
void *kmalloc(size_t size) {
    slab_alloc_t *allocator = get_default_slab_allocator();
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
    for (i = 0; i < ROUND_UP(num * size / SLAB_SIZE, SLAB_SIZE); i++)
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

#ifdef TESTS
/** returns the number of currently pages allocated
 * 
 * @return number of pages currently allocated
 */
size_t num_allocated() {
    return bitmap_count_range(&free_map, 0, free_map.bits);
}

/** returns the size of the palloc free map
 * 
 * @return size of the palloc free map
 */
size_t map_size() {
    return free_map.bits;
}
#endif