/* Implements atomic functions used in synch.c */

/* includes */
#include <atomic.h>

/* defines */

/* globals */

/* functions */

/** atomically sets dest to 1 and returns old value
 * 
 * @param dest: memory to overwrite
 * 
 * @return old value of memory at dest
 */
int test_and_set(int *dest) {
    int set = 1;
    asm volatile("lock; xchgl %0, %1" : "+r" (set), "+m" (*dest) : : "memory");
    return set;
}

/** atomically adds val to memory at dest
 * 
 * @param dest: memory to update
 * @param val: value to add
 * 
 * @return old value of memory at dest
 */
int fetch_and_add(int *dest, int val) {
    asm volatile("lock; xaddl %0, %1" : "+r" (val), "+m" (*dest) : : "memory");
    return val;
}

