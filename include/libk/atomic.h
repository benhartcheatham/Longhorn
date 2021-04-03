/* Defines atomic functions, may be eventually moved to a libc atomic file. */
#ifndef ATOMIC_H
#define ATOMIC_H

/* includes */

/* defines */

/* structs */

/* typedefs */

/* functions */
int test_and_set(int *dest);
int fetch_and_add(int *dest, int val);

#endif