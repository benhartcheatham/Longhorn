/* Defines the testing suite. */
#ifndef _TEST_H
#define _TEST_H

/* includes */
#include <stdbool.h>
#include <stdint.h>

/* defines */
#define TESTS_PER_MODULE 20

/* structs */
struct test_info {
    char name[24];
    bool expected;
    bool expression;
};

struct test_module {
    char name[50];
    uint8_t num_tests;
    struct test_info tests[TESTS_PER_MODULE];
};
/* typedefs */

/* functions */
void init_testing(bool enable_test_prints);
void RUN_ALL_TESTS(void *aux);
void test_module(struct test_module *module);
bool test(struct test_info *test);
void add_module(struct test_module *mod);
void add_test(struct test_module *mod, struct test_info test);
struct test_module make_module(char *name);
struct test_info make_test(bool expected, bool expression, char *name);
void print_test(struct test_info *test, bool verbose);

#endif