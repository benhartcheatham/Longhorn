#include <stddef.h>
#include "../libc/mem.h"

void *memset(void *dest, int c, size_t n) {
    unsigned char *d = (unsigned char *) dest;
    size_t i = 0;

    while (i < n) {
        *d = c;
        i++;
    }

    return dest;
}