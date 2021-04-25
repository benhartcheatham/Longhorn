/* Implements the page frame allocator for the kernel. This file does no mapping,
 * instead use a different page-wide allocator. */

/* includes */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <bitmap.h>
#include <kerrors.h>
#include <synch.h>
#include "../boot/multiboot.h"
#include "palloc.h"
#include "kmalloc.h"

/* defines */
#define SLAB_SIZE 64

/* structs */

/* globals */
static bitmap_t free_map;
static char *start_addr = (char *) (2*MB);
static spin_lock_t palloc_lock;

/* functions */

/** initializes kmalloc and palloc 
 * 
 * @param mb: boot record given by GRUB2
 */
void init_alloc(multiboot_info_t *mb) {
    size_t num_pages = (size_t) (((mb->mem_upper * 1024) - 2*MB) / PG_SIZE);
    bitmap_init_s(&free_map, num_pages, start_addr);
    bitmap_set_range(&free_map, 0, num_pages / PG_SIZE + 1, true);

    init_kmalloc(palloc_mult(10), PG_SIZE * 10, SLAB_SIZE);

    spin_lock_init(&palloc_lock);
}

/** gets the address of a free page frame
 * 
 * @return address of allocated page, NULL if no page exists 
 */
void *palloc() {
    return palloc_mult(1);
}

/** gets the address of cnt consecutive free page frames
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

/** frees cnt pages of memory obtained from palloc/palloc_mult
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