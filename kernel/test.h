#ifndef _TEST_H
#define _TEST_H

/* includes */
#include <stdbool.h>
#include <stdint.h>

/* defines */
#define TEST 1
#define TESTS_PER_MODULE 20

/* structs */
struct test_info {
    bool expected;
    bool expression;
    int num;
};

struct test_module {
    char *name;
    uint8_t num_tests;
    struct test_info tests[TESTS_PER_MODULE];
};
/* typedefs */

/* functions */
void init_testing(bool enable_test_prints);
void RUN_ALL_TESTS();
void test_module(struct test_module module);
bool test(bool expected, bool expression, int num);

#endif