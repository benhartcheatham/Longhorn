#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "kalloc.h"
#include "../boot/multiboot.h"
#include "../libk/bitmap.h"
#include "../libk/synch.h"

/* defines */
#define MAX_ARENA_UNIT (PG_SIZE / 4)
#define MIN_ARENA_UNIT 8
#define MEM_ARENA_SIZE (PG_SIZE / 8)
#define ARENAS_PER_PAGE (PG_SIZE / MEM_ARENA_SIZE)

/* structs */
struct mem_arena {
    char *mem;
    size_t unit;
};

struct mem_arena_g {
    bitmap_t map;
    struct mem_arena arenas[ARENAS_PER_PAGE];
};

/* static data */
static bitmap_t free_map;
static char *start_addr = (char *) (2*MB);
static struct mem_arena_g malloc_g;
static spin_lock_t malloc_lock;
static spin_lock_t palloc_lock;

/* static funcion declarations */
static size_t round_power_2(size_t n);
static size_t power_2(size_t n);

/* initalization functions */

/* initializes kmalloc and palloc */
void init_alloc(multiboot_info_t *mb) {
    size_t num_pages = (size_t) (((mb->mem_upper * 1024) - 2*MB) / PG_SIZE);
    bitmap_init_s(&free_map, num_pages, start_addr);
    bitmap_set_range(&free_map, 0, num_pages / PG_SIZE + 1, true);

    bitmap_init(&malloc_g.map, PG_SIZE);

    int i;
    int j;
    for (i = 0, j = 8; i < ARENAS_PER_PAGE; i++, j *= 2) {
        malloc_g.arenas[i].unit = j;
        malloc_g.arenas[i].mem = ((char *) malloc_g.map.map_p) + (i * MEM_ARENA_SIZE); 
    }

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
   returns -1 if addr wasn't from the memory manager and the number of pages freed otherwise */
int pfree(void *addr) {
    return pfree_mult(addr, 1);
}

/* frees cnt pages of memory obtained from the memory manager
   returns -1 if addr wasn't from the memory manager and the number of pages freed otherwise */
int pfree_mult(void *addr, size_t cnt) {
    if (spin_lock_acquire(&palloc_lock) != LOCK_ACQ_SUCC)
        return -1;
    
    size_t idx = ((char *) addr - start_addr) / PG_SIZE;

    if (idx + cnt < bitmap_get_size(&free_map) && bitmap_count_range(&free_map, idx, cnt) == cnt) {
        spin_lock_release(&palloc_lock);
        return bitmap_set_range(&free_map, idx, cnt, false);
    }

    spin_lock_release(&palloc_lock);
    return -1;
}

/* obtains the memory address of a memory area of at least size and no greater than PG_SIZE from the memory manager
   returns NULL if no memory area exists */
void *kmalloc(size_t size) {
    if (size < MIN_ARENA_UNIT)
        size = MIN_ARENA_UNIT;
    else if (size > MAX_ARENA_UNIT) {
        size += PG_SIZE - (size % PG_SIZE);
        return palloc();
    } else 
        size = round_power_2(size);

    if (spin_lock_acquire(&malloc_lock) != LOCK_ACQ_SUCC)
        return NULL;
    
    /* looks for a free slot of memory in malloc_g of size and moves up a power of 2
       if that size has no space left */
    int i;
    for (i = size; i <= MAX_ARENA_UNIT; i *= 2) {
        size_t m_idx = power_2(i) - power_2(MIN_ARENA_UNIT);
        size_t unit = malloc_g.arenas[m_idx].unit;
        size_t idx = bitmap_find_range(&malloc_g.map, m_idx * MEM_ARENA_SIZE, MEM_ARENA_SIZE, unit, false);

        if (idx != bitmap_get_size(&malloc_g.map) + 1) {
            bitmap_set_range(&malloc_g.map, idx, unit, true);
            spin_lock_release(&malloc_lock);
            return (void *) malloc_g.arenas[m_idx].mem + (idx - (m_idx * MEM_ARENA_SIZE));
        }

    }

    spin_lock_release(&malloc_lock);
    return NULL;
}

/* obtains the memory address of a zeroed memory area of at least num * size from the memory manager
   returns NULL if no memory area exists */
void *kcalloc(size_t num, size_t size) {
    //caste mem to a uint64_t since the mem kmalloc returns can't be less than 8 bytes
    uint64_t *mem = (uint64_t *) kmalloc(num * size);
    size_t i;
    for (i = 0; i < round_power_2(num * size) / 8; i++)
        mem[i] = 0;
    
    return (void *) mem;
}

/* frees a unit of memory gotten from kmalloc or kcalloc
   returns -1 if memory wasn't obtained from kmalloc or kcalloc and num bytes freed otherwise */
int kfree(void *addr) {
    uint32_t a_addr = (uint32_t) addr;
    //round addr down to an arena mem address
    a_addr = a_addr - (a_addr % MEM_ARENA_SIZE);

    if (spin_lock_acquire(&malloc_lock) != LOCK_ACQ_SUCC)
        return -1;
    
    int i;
    for (i = 0; i < ARENAS_PER_PAGE; i++)
        if (malloc_g.arenas[i].mem == (char *) a_addr) {
            uint32_t idx = a_addr % MEM_ARENA_SIZE;
            bitmap_set_range(&malloc_g.map, i * MEM_ARENA_SIZE + idx, malloc_g.arenas[i].unit, false);
            spin_lock_release(&malloc_lock);
            return malloc_g.arenas[i].unit;
        }
    
    spin_lock_release(&malloc_lock);
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