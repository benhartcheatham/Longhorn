/* Convenience include for the mem* functions 
 * These functions are normally put in string.h */
#ifndef _MEM_H
#define _MEM_H

/* includes */
#include <stddef.h>

/* defines */

/* structs */

/* typedefs */

/* functions */
extern int memcmp(const void *p1, const void *p2, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern void *memset(void *dest, int c, size_t n);

#endif