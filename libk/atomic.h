#ifndef ATOMIC_H
#define ATOMIC_H

int test_and_set(int *dest);
int fetch_and_add(int *dest, int val);

#endif