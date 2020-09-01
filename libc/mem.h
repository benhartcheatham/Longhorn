/* Convenience include for the mem* functions 
   These functions are normally put in string.h */

#include <stddef.h>

int memcmp(const void *p1, const void *p2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);