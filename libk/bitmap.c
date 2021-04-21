/* A bitmap implementation
 * The bitmap consists of an array of ELEM_TYPE values
 * bits are modified in the array to represent a bitmap */ 

/* includes */
#include <bitmap.h>
#include "../kernel/palloc.h"

/* defines */

/* globals */

/* functions */

/* BIT UTILITY FUNCTIONS */

/** returns a bit mask with 1 at idx and 0 in every other bit
 * map is size of ELEM_BITS
 * 
 * @param idx: index to mask against
 * 
 * @return bit mask with 1 at idx and 0 in every other bit
 */
static inline ELEM_TYPE bit_mask(size_t idx) {
    return (ELEM_TYPE) 1 << (idx % ELEM_BITS);
}

/** returns the element that contains map_idx
 * 
 * @param map_idx: index of bit in map
 * 
 * @return element that contains map_idx
 */
static inline size_t elem_idx(size_t map_idx) {
    return map_idx / ELEM_BITS;
}

/** returns the number of map elements required to represent num_bits
 * 
 * @param num_bits: number of bits to represent
 * 
 * @return number of map elements required to represent num_bits
 */
static inline size_t num_elems(size_t num_bits) {
    return (size_t) (num_bits + (num_bits - (num_bits % ELEM_BITS))) / ELEM_BITS;
}

/** returns the number of bytes required to represent num_bits
 * 
 * @param num_bits: number of bits to represent
 * 
 * @return number of bytes required to represent num_bits
 */
static inline size_t num_bytes(size_t num_bits) {
    return (size_t) ELEM_SIZE * num_elems(num_bits);
}

/* INITIALIZATION FUNCTIONS */

/** Initializes a bitmap m of size bits
 * the actual map is allocated by palloc
 * 
 * @param m: bitmap struct to initialize
 * @param bits: size of the map
 */
void bitmap_init(bitmap_t *m , size_t bits) {
    m->bits = bits;
    m->map_p = (ELEM_TYPE *) palloc();
}

/** Initializes a bitmap of size bits at m that corresponds to the area at map_p
 * Use if palloc is not safe to use
 * 
 * @param m: bitmap struct ot initialize
 * @param bits: size of the map
 * @param map_p: memory store the map in
 */
void bitmap_init_s(bitmap_t *m, size_t bits, void *map_p) {
    m->bits = bits;
    m->map_p = (ELEM_TYPE *) map_p;
}

/* BIT MANIPULATION FUNCTIONS */

/** sets the bit at idx to true
 * 
 * @param m: bitmap to index into
 * @param idx: index of bit to set top true
 */
void bitmap_set(bitmap_t *m, size_t idx) {
    m->map_p[elem_idx(idx)] |= bit_mask(idx);
}

/** sets the bit at idx to false
 * 
 * @param m: bitmap to index into
 * @param idx: index of bit to set to false
 */
void bitmap_reset(bitmap_t *m, size_t idx) {
    m->map_p[elem_idx(idx)] &= (~bit_mask(idx));
}

/** gets the value of the bit at idx
 * 
 * @param m: bitmap to index into
 * @param idx: index of bit to get value of
 * 
 * @return value of bit at idx
 */
bool bitmap_test(bitmap_t *m, size_t idx) {
    return (m->map_p[elem_idx(idx)] & bit_mask(idx)) != 0;
}

/** sets the bit at idx to val
 * 
 * @param m: bitmap to index into
 * @param idx: index of bit to set
 * @param val: value to set bit at idx to
 */
void bitmap_set_val(bitmap_t *m, size_t idx, bool val) {
    if (val)
        bitmap_set(m, idx);
    else
        bitmap_reset(m, idx);
}

/* BITMAP-WIDE FUNCTIONS */

/** sets all bits in the bitmap to val
 * 
 * @param m: bitmap to index into
 * @param val: value to set bits in m to
 */
void bitmap_set_all(bitmap_t *m, bool val) {
    ELEM_TYPE new_val = val ? (ELEM_TYPE) (~0) : (ELEM_TYPE) 0;

    size_t i;
    for (i = 0; i < num_elems(m->bits); i++)
        m->map_p[i] = new_val;
    
}

/** sets the range [start, start + len) to val
 * 
 * @param m: bitmap to index into
 * @param start: starting index of range
 * @param len: size of range
 * @param val: value to set bits in range to
 * 
 * @return number of bits set to val in range
 */
size_t bitmap_set_range(bitmap_t *m, size_t start, size_t len, bool val) {
    size_t i;
    for (i = start; i < start + len; i++)
        bitmap_set_val(m, i, val);
    
    return i;
}

/** counts the number of bits set to true in the range [start, start + len)
 * 
 * @param m: bitmap to index into
 * @param start: starting index of range
 * @param len: size of range
 * 
 * @return number of bits set to true in range
 */
size_t bitmap_count_range(bitmap_t *m, size_t start, size_t len) {
    size_t cnt = 0;

    size_t i;
    for (i = start; i < start + len; i++)
        if (bitmap_test(m, i))
            cnt++;
    
    return cnt;
}

/** finds the index where the next len bits are set to val
 * 
 * @param m: bitmap to index into
 * @param start: index to start search at
 * @param len: size of range to search
 * @param count: number of bits to look for that are set to val
 * @param val: value to search for
 * 
 * @return starting index of range, number of bits in m + 1 if no such range exists */
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

/** returns the number of bits in the bitmap
 * 
 * @param m: bitmap to get size of
 * 
 * @return size of bitmap in bits
 */
size_t bitmap_get_size(bitmap_t *m) {
    return m->bits;
}