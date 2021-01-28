#include <stddef.h>
#include <mem.h>


void *memcpy(void *dest, const void *src, size_t n) {
    char *dest_char = (char *) dest;
    char *src_char = (char *) src;

    size_t i;
    for (i = 0; i < n; i++)
        dest_char[i] = src_char[i];

    return dest;
}