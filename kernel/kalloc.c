#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "kalloc.h"
#include "../boot/multiboot.h"
#include "../libk/bitmap.h"

/* static data */
static bitmap_t free_map;
static char *start_addr = (char *) (2*MB);

void init_alloc(multiboot_info_t *mb) {
    size_t num_pages = (size_t) (((mb->mem_upper * 1024) - 2*MB) / PG_SIZE);
    bitmap_init_s(&free_map, num_pages, start_addr);
}

void *palloc() {
    return palloc_mult(1);
}

void *palloc_mult(size_t cnt) {
    size_t idx = bitmap_find_range(&free_map, 0, bitmap_get_size(&free_map), cnt, false);

    if (idx != bitmap_get_size(&free_map) + 1) {
        bitmap_set_range(&free_map, idx, cnt, true);
        return (void *) (start_addr + (idx * PG_SIZE));
    }
    
    return NULL;
}

int pfree(void *addr) {
    return pfree_mult(addr, 1);
}

int pfree_mult(void *addr, size_t cnt) {
    size_t idx = ((char *) addr - start_addr) / PG_SIZE;

    if (idx + cnt < bitmap_get_size(&free_map) && bitmap_count_range(&free_map, idx, cnt) == cnt)
        return bitmap_set_range(&free_map, idx, cnt, false);

    return -1;
}

void *kmalloc(size_t size) {
    return NULL;
}

void *kcalloc(size_t num, size_t size) {
    return NULL;
}

int kfree(void *addr) {
    return -1;
}