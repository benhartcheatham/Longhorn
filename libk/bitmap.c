/* 
 * A bitmap implementation
 * The bitmap consists of an array of ELEM_TYPE values
 * bits are modified in the array to represent a bitmap
 * 
 */ 

#include "bitmap.h"

/* BIT UTILITY FUNCTIONS */

/* returns a bit mask with 1 at idx and 0 in every other bit
   map is size of ELEM_BITS */
static inline ELEM_TYPE bit_mask(size_t idx) {
    return (ELEM_TYPE) 1 << (idx % ELEM_BITS);
}

/* returns the element that notains map_idx */
static inline size_t elem_idx(size_t map_idx) {
    return map_idx / ELEM_BITS;
}

/* returns the number of map elements required to represent num_bits */
static inline size_t num_elems(size_t num_bits) {
    return (size_t) (num_bits + (num_bits - (num_bits % ELEM_BITS))) / ELEM_BITS;
}

/* returns the number of bytes required for num_bits */
static inline size_t num_bytes(size_t num_bits) {
    return (size_t) ELEM_SIZE * num_elems(num_bits);
}

/* INITIALIZATION FUNCTIONS */

//TODO: implement
void bitmap_init(bitmap_t *m __attribute__ ((unused)), size_t bits __attribute__ ((unused))) {
    return;
}

/* Initializes a bitmap at m that corresponds to the area at map_p */
void bitmap_init_s(bitmap_t *m, size_t bits, void *map_p) {
    m->bits = bits;
    m->map_p = (ELEM_TYPE *) map_p;
}

/* BIT MANIPULATION FUNCTIONS */

/* Sets the bit at idx to true */
void bitmap_set(bitmap_t *m, size_t idx) {
    m->map_p[elem_idx(idx)] |= bit_mask(idx);
}

/* Sets the bit at idx to false */
void bitmap_reset(bitmap_t *m, size_t idx) {
    m->map_p[elem_idx(idx)] &= (~bit_mask(idx));
}

/* Returns the value of bit at idx */
bool bitmap_test(bitmap_t *m, size_t idx) {
    return (m->map_p[elem_idx(idx)] & bit_mask(idx)) != 0;
}

/* Sets the bit at idx to val */
void bitmap_set_val(bitmap_t *m, size_t idx, bool val) {
    if (val)
        bitmap_set(m, idx);
    else
        bitmap_reset(m, idx);
}

/* BITMAP-WIDE FUNCTIONS */

/* sets all bits in the bitmap to val */
void bitmap_set_all(bitmap_t *m, bool val) {
    ELEM_TYPE new_val = val ? (ELEM_TYPE) (~0) : (ELEM_TYPE) 0;

    size_t i;
    for (i = 0; i < num_elems(m->bits); i++)
        m->map_p[i] = new_val;
    
}

/* sets the range [start, start + len) to val, returns num bits set to val*/
size_t bitmap_set_range(bitmap_t *m, size_t start, size_t len, bool val) {
    size_t i;
    for (i = start; i < start + len; i++)
        bitmap_set_val(m, i, val);
    
    return i;
}

/* counts the number of bits set to true in the range [start, start + len) */
size_t bitmap_count_range(bitmap_t *m, size_t start, size_t len) {
    size_t cnt = 0;

    size_t i;
    for (i = start; i < start + len; i++)
        if (bitmap_test(m, i))
            cnt++;
    
    return cnt;
}

/* Finds the index where the next len bits are set to val
   returns m->bits + 1 if no such range exists */
size_t bitmap_find_range(bitmap_t *m, size_t start, size_t len, size_t count, bool val) {
    size_t begin = start;
    size_t cnt = 0;

    size_t i;
    for (i = start; i < start + len; i++) {
        if (bitmap_test(m, i) == val)
            cnt++;
        else {
            cnt = 0;
            begin = i + 1;
        }

        if (cnt == count)
            return begin;
    }

    return m->bits + 1;
}

/* Returns the number of bits in the bitmap */
size_t bitmap_get_size(bitmap_t *m) {
    return m->bits;
}