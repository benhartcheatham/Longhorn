#ifndef ATOMIC_H
#define ATOMIC_H

int atomic_test_and_set(unsigned int *dest, unsigned int val);
int atomic_add(unsigned int *dest, unsigned int val);
int atomic_sub(unsigned int *dest, unsigned int val);

#endif