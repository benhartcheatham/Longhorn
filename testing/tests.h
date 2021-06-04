/* Defines the testing suite. Tests are run in "test groups", or collections of tests testing one feature. 
 * To add a test group to be tested, a function called init_<feature>_group needs to be made, that has the type 
 * init_group_function, and added to the init_testing() function in tests.c. Each test group can specify a 
 * setup and teardown function that will be called before and after the tests in the group are run. For setting up
 * a test group, the macro TEST_GROUP_INIT is provided, but tests have to be manually. If one test in a test
 * group fails, the succeeding tests will not be run and the teardown function will be called.
 * 
 * Tests in a test group have to have the test_function type, and must return true at the end of the test, otherwise
 * they will fail automatically. The macros CHECK_EQ and CHECK_NEQ will check for equality of arguments and inequality of 
 * arguments respectively, and fail the test automatically if the comparison fails. The macro FAIL_TEST will automatically
 * fail a test.
 */
#ifndef _TESTS_H
#define _TESTS_H

/* includes */
#include <stdbool.h>
#include <stdint.h>
#include <kerrors.h>

/* global data */
char test_err_msg[256];

/* defines */
#define MAX_NUM_TESTS 20
#define MAX_NUM_TEST_GROUPS 50
#define TEST_GROUP_INIT(n, set, tear)                              \
(test_group) {.name = n, .setup = (setup_function) set,            \
              .teardown = (teardown_function) tear}

#define FAIL_TEST() { return false; }

#define CHECK_EQ(a, b, msg) {                                           \
        if (a != b) {                                                   \
            sprintf(test_err_msg, "CHECK_EQ FAILED: %s", (char *) msg); \
            return false;                                               \
        }                                                               \
    }                                                                   

#define CHECK_NEQ(a, b, msg) {                                           \
        if (a == b) {                                                    \
            sprintf(test_err_msg, "CHECK_NEQ FAILED: %s", (char *) msg); \
            return false;                                                \
        }                                                                \
    }                                                                    

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
    char *name;
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