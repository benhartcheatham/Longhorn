/* Tests the process/thread subsystem */

/* includes */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <list.h>
#include "tests.h"
#include "../kernel/proc.h"

/* defines */
#define NUM_PROC_TESTS 2

/* globals */
static bool test_create(void);
static bool test_kill(void);

static void proc_test_func(void *aux);

static test_group proc_test_group;

/* functions */

/** makes the process test group
 * 
 * @return the initialized test group, with tests added
 */
test_group *init_proc_group(void) {
    proc_test_group = TEST_GROUP_INIT("Processes", NULL, NULL);

    test_function test_funcs[NUM_PROC_TESTS] = {test_create, test_kill};
    char *test_names[NUM_PROC_TESTS] = {"create1", "kill"};
    for (int i = 0; i < NUM_PROC_TESTS; i++)
        add_test(&proc_test_group, test_funcs[i], test_names[i]);
    
    return &proc_test_group;
}

/** tests proc_create
 * 
 * @return false if test fails, true if test passes
 */
static bool test_create(void) {
    struct process *p1 = proc_create("test", proc_test_func, NULL);
    CHECK_EQ(p1->pid, 1, "create process");
    CHECK_EQ(p1->num_live_threads, 1, "num live threads does not equal 1");

    int ret = 0;
    proc_kill(p1, &ret);

    return true;
}

/** tests proc_kill
 * 
 * @return false if test fails, true if test passes
 */
static bool test_kill(void) {
    struct process *p1 = proc_create("test", proc_test_func, NULL);

    int ret = 0;
    proc_kill(p1, &ret);
    CHECK_EQ(ret, 0, "killing process p1 returned an error code");

    return true;
}

/** function used for testing processes
 * 
 * @param aux: unused
 */
static void proc_test_func(void *aux __attribute__ ((unused))) {
    while (1) {}
    return;
}