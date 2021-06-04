/* Defines the testing suite. */
#ifndef _TEST_H
#define _TEST_H

/* includes */
#include <stdbool.h>
#include <stdint.h>
#include <kerrors.h>

// i don't really like this
char test_err_msg[256];

/* defines */
#define MAX_NUM_TESTS 20
#define MAX_NUM_TEST_GROUPS 50

#define FAIL_TEST() { return false; }

#define CHECK_EQ(a, b, msg) {                                  \
        if (a != b) {                                          \
            sprintf(test_err_msg, "CHECK_EQ FAILED: %s", (char *) msg); \
            return false;                                      \
        }                                                      \
    }                                                          \

#define CHECK_NEQ(a, b, msg) {                                 \
        if (a == b) {                                          \
            sprintf(test_err_msg, "CHECK_NEQ FAILED: %s", (char *) msg); \
            return false;                                      \
        }                                                      \
    }                                                          \

/* typedefs */
typedef bool (*test_function)(void);
typedef void (*setup_function)(void);
typedef void (*teardown_function)(void);

/* structs */
typedef struct {
    int num_tests;
    char *test_names[MAX_NUM_TESTS];
    test_function tests[MAX_NUM_TESTS];
    setup_function setup;
    teardown_function teardown;
    char name[24];
} test_group;

typedef test_group * (*init_group_function)(void);

/* functions */
void init_testing();
void run_groups(void *aux);
void add_group(init_group_function testg);
void add_test(test_group *g, test_function test, char *test_name);
void print_test(char *test_name, bool passed);

/* test group functions */
test_group *init_slab_group(void);
test_group *init_proc_group(void);

#endif