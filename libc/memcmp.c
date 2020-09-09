#include <stddef.h>
#include "../libc/mem.h"

int memcmp(const void *p1, const void *p2, size_t n) {
    const char *a = (const char *) p1;
    const char *b = (const char *) p2;

    size_t i;
    for (i = 0; i < n; i++)
        if (*a - *b != 0)
            return *a - *b;

    return 0;
}