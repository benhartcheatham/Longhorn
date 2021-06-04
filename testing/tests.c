/* File for the testing subsystem of the kernel. Look to tests.h for how to use the subsytem */

/* includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <kerrors.h>
#include "tests.h"
#include "../drivers/display.h"
#include "../kernel/port_io.h"

/* defines */

/* globals */
int num_groups = 0;
test_group *groups[MAX_NUM_TEST_GROUPS];

/* prototypes */
static void run_test_group(test_group *g);

/* functions */

/** initializes all tests for the testing subsystem of the kernel
 * all tests should be in this function
 * 
 */
void init_testing() {
    add_group(init_slab_group);
    add_group(init_proc_group);
}

/** adds a group to be tested
 * 
 * @param testg: function to initalize a test group,
 *               the test group is expected to already contain tests
 */
void add_group(init_group_function testg) {
    if (num_groups >= MAX_NUM_TEST_GROUPS)
        return;
    
    groups[num_groups] = testg();
    num_groups++;
}

/** adds a test to the given test group
 * 
 * @param g: test group to add test to
 * @param test: test to add
 * @param test_name: name of the test
 */
void add_test(test_group *g, test_function test, char *test_name) {
    if (g->num_tests >= MAX_NUM_TESTS)
        return;

    g->tests[g->num_tests] = test;
    g->test_names[g->num_tests] = test_name;
    g->num_tests++;
}


/** runs all tests in every test group, called in kmain() if testing is enabled
 * 
 * @param aux: unused
 */
void run_groups(void *aux __attribute__ ((unused))) {
    kprintf("NUMBER TEST GROUPS: %d\n", num_groups);
    for (int i = 0; i < num_groups; i++)
        run_test_group(groups[i]);

    kprintf("TESTING COMPLETED\nexiting...\n");
    outw(0x604, 0x2000);    // issues shutdown command to QEMU
}

/** runs the tests in the given test_module
 * 
 * @param g: test group to run
 */
static void run_test_group(test_group *g) {
    kprintf("\nTESTING GROUP: %s\n", g->name);

    if (g->setup != NULL)
        g->setup();

    int i;
    for (i = 0; i < MAX_NUM_TESTS && i < g->num_tests; i++) {
        bool result = g->tests[i]();
        print_test(g->test_names[i], result);

        if (result == false) {
            kprintf("FAILED ON TEST %s... NUM PASSED: %d NUM FAILED: %d\n", g->test_names[i], i, g->num_tests - i);
            goto teardown;
            return;
        }
    }

    kprintf("TESTS DONE FOR GROUP: %s\n\n", g->name);
teardown:
    if (g->teardown != NULL)
        g->teardown();
    return;
}

/** prints the result of a test
 * 
 * @param test_name: name of test
 * @param passed: whether the test passed
 */
void print_test(char *test_name, bool passed) {
    
    kprintf("%s", test_name);

    // align the status of test output using spaces
    char padding[28];
    memset(padding, 0, 28);
    int i = 0;
    for (i = 0; i < 28 - (int) strlen(test_name); i++)
        padding[i] = ' ';
    padding[i] = '\0';

    kprintf("%s", padding);
// if TESTS is defined use ANSI codes to color test results in terminal
#ifdef TESTS
    if (passed) {
        kprintf("[\033[01;32mSUCCESS\033[00;37m]\n");
    } else {
        kprintf("[\033[01;31mFAIL\033[00;37m]\n");
        kprintf("\t%s\n", test_err_msg);
        memset(test_err_msg, 0, sizeof(test_err_msg));
    }
#else
    struct display *dis = get_default_dis_driver();
    if (passed) {
        kprintf("[");
        dis->dis_setcol(0x23ca23, 0x000000);
        kprintf("SUCCESS");
        dis->dis_setcol(0xffffff, 0x000000);
        kprintf("]\n");
    } else {
        kprintf("[");
        dis->dis_setcol(0xec4949, 0x000000);
        kprintf("FAIL");
        dis->dis_setcol(0xffffff, 0x000000);
        kprintf("]\n");

        kprintf("\t%s\n", test_err_msg);
        memset(test_err_msg, 0, sizeof(test_err_msg));
    }
#endif

}