/* Defines functionality of the default bitmap implemenation. 
 * NOTE: Needs to be updated to use function pointers. */
#ifndef _BITMAP_H
#define _BITMAP_H

/* includes */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

/* defines */
#define ELEM_TYPE uint32_t
#define ELEM_SIZE sizeof(uint32_t)
#define ELEM_BITS (ELEM_SIZE * CHAR_BIT)

/* structs */
struct bitmap {
    ELEM_TYPE *map_p;   // pointer to the free map
    size_t bits;    // number of bits in the map
};

/* typedefs */
typedef struct bitmap bitmap_t;

/* functions */

/* init functions */
void bitmap_init(bitmap_t *m, size_t bits);
void bitmap_init_s(bitmap_t *m, size_t bits, void *map_p);

/* bit manipulation functions */
void bitmap_set(bitmap_t *m, size_t idx);
void bitmap_reset(bitmap_t *m, size_t idx);
bool bitmap_test(bitmap_t *m, size_t idx);
void bitmap_set_val(bitmap_t *m, size_t idx, bool val);

/* bitmap-wide functions */
void bitmap_set_all(bitmap_t *m, bool val);
size_t bitmap_set_range(bitmap_t *m, size_t start, size_t len, bool val);
size_t bitmap_count_range(bitmap_t *m, size_t start, size_t len);
size_t bitmap_find_range(bitmap_t *m, size_t start, size_t len, size_t count, bool val);

/* bitmap_t functions */
size_t bitmap_get_size(bitmap_t *m);

#endif