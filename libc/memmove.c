#include <stddef.h>
#include "../libc/mem.h"

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