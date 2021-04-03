/* Implementation of memmove. */

/* includes */
#include <stddef.h>
#include <mem.h>

/* defines */

/* globals */

/* functions */

/** moves n bytes from src to dest
 * 
 * @param dest: destination pointer
 * @param src: source pointer
 * @param n: number of bytes to move
 *
 * @return pointer to start of where memory was moved to
 */
void *memmove(void *dest, const void *src, size_t n) {

    if (dest < src)
        return memcpy(dest, src, n);
    else {
        char *d = (char *) dest;
        const char *s = (char *) src;

        size_t i;
        for (i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }

    return dest;
}