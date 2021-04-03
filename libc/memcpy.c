/* Implementation of memcpy. */

/* includes */
#include <stddef.h>
#include <mem.h>

/* defines */

/* globals */

/* functions */

/** copies n bytes at src to dest
 * 
 * @param dest: destination pointer
 * @param src: source pointer
 * @param n: number of bytes to copy
 *
 * @return pointer to the start of copied memory
 */
void *memcpy(void *dest, const void *src, size_t n) {
    char *dest_char = (char *) dest;
    char *src_char = (char *) src;

    size_t i;
    for (i = 0; i < n; i++)
        dest_char[i] = src_char[i];

    return dest;
}