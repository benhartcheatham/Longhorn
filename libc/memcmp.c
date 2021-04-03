/* Implementation of memcmp. */

/* includes */
#include <stddef.h>
#include <mem.h>

/* defines */

/* globals */

/* functions */

/** compares n bytes from p2 to p1
 * 
 * @param p1: destination pointer
 * @param p2: source pointer
 * @param n: number of bytes to copy
 * 
 * @return difference between first differing byte (p1 - p2), or 0 if bytes don't differ
 */
int memcmp(const void *p1, const void *p2, size_t n) {
    const char *a = (const char *) p1;
    const char *b = (const char *) p2;

    size_t i;
    for (i = 0; i < n; i++)
        if (*a - *b != 0)
            return *a - *b;

    return 0;
}