#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "kalloc.h"
#include "../boot/multiboot.h"
#include <bitmap.h>
#include <list.h>
#include <synch.h>
#include <kerrors.h>
#include <slab.h>

/* defines */
#define ROUND_UP(x, size) (((x + size - 1) / size) * size)

/* structs */
struct allocation {
    void *addr;
    size_t size;

    struct list_node node;
};

/* static data */
static bitmap_t free_map;
static char *start_addr = (char *) (4*MB);
static spin_lock_t malloc_lock;
static spin_lock_t palloc_lock;
static list allocations;

/* static funcion declarations */
static size_t round_power_2(size_t n);
static size_t power_2(size_t n);

/* initalization functions */

/* initializes kmalloc and palloc */
void init_alloc(multiboot_info_t *mb) {
    size_t num_pages = (size_t) (((mb->mem_upper * 1024) - 4*MB) / PG_SIZE);
    bitmap_init_s(&free_map, num_pages, start_addr);
    bitmap_set_range(&free_map, 0, num_pages / PG_SIZE + 1, true);

    list_init(&allocations);
    slab_init(get_default_slab_allocator(), palloc_mult(10), 10 * PG_SIZE, sizeof(uint64_t), NULL);

    spin_lock_init(&malloc_lock);
    spin_lock_init(&palloc_lock);
}

/* gets the address of a free page of memory from the memory manager 
   returns NULL if no free pages exist */
void *palloc() {
    return palloc_mult(1);
}

/* gets the address of cnt consecutive free pages of memory from the memory manager
   returns NULL if this range doesn't exist */ 
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

/* frees a page of memory obtained from the memory manager
   returns -MEM_ALLOC_FAIL if addr wasn't from the memory manager and the number of pages freed otherwise */
int pfree(void *addr) {
    return pfree_mult(addr, 1);
}

/* frees cnt pages of memory obtained from the memory manager
   returns -MEM_FREE_FAIL if addr wasn't from the memory manager and the number of pages freed otherwise */
int pfree_mult(void *addr, size_t cnt) {
    if (spin_lock_acquire(&palloc_lock) != LOCK_ACQ_SUCC)
        return -MEM_FREE_FAIL;
    
    size_t idx = ((char *) addr - start_addr) / PG_SIZE;

    if (idx + cnt < bitmap_get_size(&free_map) && bitmap_count_range(&free_map, idx, cnt) == cnt) {
        spin_lock_release(&palloc_lock);
        return bitmap_set_range(&free_map, idx, cnt, false);
    }

    spin_lock_release(&palloc_lock);
    return -MEM_FREE_FAIL;
}

/* obtains the memory address of a memory area of at least size and no greater than PG_SIZE from the memory manager
   returns NULL if no memory area exists */
void *kmalloc(size_t size) {

    return NULL;
}

/* obtains the memory address of a zeroed memory area of at least num * size from the memory manager
   returns NULL if no memory area exists */
void *kcalloc(size_t num, size_t size) {
    //cast mem to a uint64_t since the mem kmalloc returns can't be less than 8 bytes
    uint64_t *mem = (uint64_t *) kmalloc(num * size);
    size_t i;
    for (i = 0; i < round_power_2(num * size) / 8; i++)
        mem[i] = 0;
    
    return (void *) mem;
}

/* frees a unit of memory gotten from kmalloc or kcalloc
   returns -MEM_FREE_FAIL if memory wasn't obtained from kmalloc or kcalloc and num bytes freed otherwise */
int kfree(void *addr) {
    
    return pfree(addr);
}

/* returns the number of pages allocated */
size_t num_allocated() {
    return bitmap_count_range(&free_map, 0, free_map.bits);
}

/* returns the size of the free map */
size_t map_size() {
    return free_map.bits;
}

/* static functions */

/* returns n rounded up to the next power of 2, or the next power of 2 
   that is greater than or equal to n */
static size_t round_power_2(size_t n) {
    if ((n && !(n & (n - 1))) || n == 0)
        return n;
    
    size_t pow = 1;

    while (pow < n)
        pow <<= 1;
    
    return pow;
}

/* returns the power of 2 of n
   if n isn't a power of 2, n is rounded to the closest power of 2 less than n */
static size_t power_2(size_t n) {
    size_t pow = 0;

    while (n > 0) {
        n /= 2;
        pow++;
    }

    return pow;
}