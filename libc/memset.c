/* Implementation of memset. */

/* includes */
#include <stddef.h>
#include <mem.h>

/* defines */

/* globals */

/* functions */

/** sets n bytes at dest to c
 * 
 * @param dest: destination pointer
 * @param c: value to set
 * @param n: number of bytes to set
 * 
 * @return pointer to memory that was set
 */
void *memset(void *dest, int c, size_t n) {
    unsigned char *d = (unsigned char *) dest;
    size_t i = 0;

    while (i < n) {
        d[i] = (unsigned char) c;
        i++;
    }

    return dest;
}