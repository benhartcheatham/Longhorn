#include "atomic.h"

int test_and_set(int *dest) {
    int set = 1;
    asm volatile("lock; xchgl %0, %1" : "+r" (set), "+m" (*dest) : : "memory");
    return set;
}

int fetch_and_add(int *dest, int val) {
    asm volatile("lock; xaddl %0, %1" : "+r" (val), "+m" (*dest) : : "memory");
    return val;
}

