/* Tests the slab allocator */

/* includes */
#include <stdbool.h>
#include <slab.h>
#include <string.h>
#include <stdio.h>
#include "tests.h"
#include "../kernel/kalloc.h"

/* defines */
#define NUM_SLAB_TESTS 2

/* globals */
static void slab_setup(void);
static void slab_teardown(void);

static bool test_slab1(void);
static bool test_slab2(void);

static test_group slab_test_group;

void *slab_alloc_mem = NULL;

/* functions */

/** initializes the slab allocator test group
 * 
 * @return initialized slab allocator test group, with tests added
 */
test_group *init_slab_group(void) {
    slab_test_group = TEST_GROUP_INIT("Slab Allocator", slab_setup, slab_teardown);

    test_function test_funcs[NUM_SLAB_TESTS] = {test_slab1, test_slab2};
    char *test_names[NUM_SLAB_TESTS] = {"slab1", "slab2"};
    for (int i = 0; i < NUM_SLAB_TESTS; i++)
        add_test(&slab_test_group, test_funcs[i], test_names[i]);
    
    return &slab_test_group;
}

/** tests a small allocation
 * 
 * @return false if test fails, true if test passes
 */
static bool test_slab1(void) {
    slab_alloc_t *slab_allocator = get_default_slab_allocator();
    void *slab_ret = NULL;

    CHECK_NEQ(slab_alloc_mem, NULL, "mem allocation for slab failed");

    slab_ret = slab_allocator->alloc(slab_allocator, 1);
    CHECK_NEQ(slab_ret, NULL, "allocation 1");

    CHECK_EQ(slab_allocator->free(slab_allocator, slab_ret, 1), SLAB_SUCC, "Slab allocator free 1");

    return true;
}

/** tests a normal allocation
 * 
 * @return false if test fails, true if test passes
 */
static bool test_slab2(void) {
    slab_alloc_t *slab_allocator = get_default_slab_allocator();
    void *slab_ret = NULL;

    slab_ret = slab_allocator->alloc(slab_allocator, 30);
    CHECK_NEQ(slab_ret, NULL, "allocation 2");

    CHECK_EQ(slab_allocator->free(slab_allocator, slab_ret, 30), SLAB_SUCC, "Slab allocator free 2");

    return true;
}

/** allocates memory for the slab allocator tests */
static void slab_setup(void) {
    slab_alloc_mem = palloc();
}

/** frees memory used by the slab allocator tests */
static void slab_teardown(void) {
    kfree(slab_alloc_mem);
}