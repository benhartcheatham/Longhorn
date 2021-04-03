/* File for testing parts of the kernel. To make a test system, create a test module then fill the module in with tests.
 * To test the module, add the module to the test suite. All registered test modules are tested in the order they are added in. */

/* includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <list.h>
#include <slab.h>
#include <kerrors.h>
#include "test.h"
#include "kalloc.h"
#include "proc.h"
#include "thread.h"

/* defines */
#define NUM_MODULES 10

/* globals */
static bool test_prints = false;
static uint32_t num_modules_made = 0;
struct test_module modules[NUM_MODULES];

/* prototypes */
void proc_test_func(void *aux);

/* functions */

/** initializes all tests for the testing module of the kernel
 * all tests should be in this function
 * 
 * @param enable_test_prints: whether the result of tests should be printed
 */
void init_testing(bool enable_test_prints) {
    test_prints = enable_test_prints;
    slab_alloc_t *slab_allocator = get_default_slab_allocator();
    printf("slab allocator addr: %x\n", slab_allocator->mem);
    void *slab_alloc_mem = NULL;
    void *slab_ret = NULL;
    void *slab_ret2 = NULL;

    struct test_module kalloc = make_module("Kalloc");
    // the + 6 is 1 for the free_map, 1 for malloc arena, and 4 for starting procs and threads 
    #ifdef TESTS
    add_test(&kalloc, make_test(true, num_allocated() == (map_size() / PG_SIZE + 8), "Initialization"));
    #endif
    add_test(&kalloc, make_test(true, (slab_alloc_mem = palloc()) != NULL, "Allocate mem for slab allocator"));
    add_module(&kalloc);

    struct test_module slab_alloc = make_module("Slab Alloc");
    add_test(&slab_alloc, make_test(true, slab_init(slab_allocator, slab_alloc_mem, 4096, 32, NULL) == SLAB_SUCC, "slab alloctor initialization"));
    
    slab_ret = slab_allocator->alloc(slab_allocator, 1);
    add_test(&slab_alloc, make_test(true, slab_ret != NULL, "Slab allocator allocation 1"));
    slab_ret2 = slab_allocator->alloc(slab_allocator, 30);
    add_test(&slab_alloc, make_test(true, slab_ret2 != NULL, "Slab allocator allocation 2"));
    add_test(&slab_alloc, make_test(true, slab_allocator->free(slab_allocator, slab_ret, 1) == SLAB_SUCC, "Slab allocator free 1"));
    add_test(&slab_alloc, make_test(true, slab_allocator->free(slab_allocator, slab_ret2, 30) == SLAB_SUCC, "Slab allocator free 2"));
    add_module(&slab_alloc);

    // struct test_module procs = make_module("Processes");
    // int pid = proc_create("test", proc_test_func, NULL);
    // add_test(&procs, make_test(true, pid == 2, "Create1"));
    // add_test(&procs, make_test(true, num_allocated() == (map_size() / PG_SIZE + 8), "Create2"));
    // add_test(&procs, make_test(true, num_threads() == 3, "Create3"));

    // const list_node *proc_node = proc_peek_all_list();
    // while (proc_node != NULL && LIST_ENTRY(proc_node, struct process, node)->pid != pid)
    //     proc_node = proc_node->next;
    // add_module(&procs);
}

/** function used for testing processes
 * 
 * @param aux: unused
 */
void proc_test_func(void *aux __attribute__ ((unused))) {
    while (1) {}
    return;
}

/** adds a module to the testing system
 * 
 * @param mode: module to add
 */
void add_module(struct test_module *mod) {
    if (num_modules_made >= NUM_MODULES)
        return;
    
    modules[num_modules_made] = *mod;
    num_modules_made++;
}

/** adds a test to the given module
 * 
 * @param mod: module to add test to
 * @param test: test to add
 */
void add_test(struct test_module *mod, struct test_info test) {
    if (mod->num_tests >= TESTS_PER_MODULE)
        return;
    
    mod->tests[mod->num_tests] = test;
    mod->num_tests++;
}

/** creates a module
 * 
 * @param name: name of the module
 *
 * @return the resulting test module
 */
struct test_module make_module(char *name) {
    struct test_module m;

    if (strlen(name) < 49)
        memcpy(m.name, name, strlen(name));
    else
        memcpy(m.name, name, 49);
    m.num_tests = 0;

    return m;
}

/** creates a test
 * 
 * @param expected: expected result
 * @param expression: expression to test
 * @param name: name of the test
 *
 * @return the resulting test
 */
struct test_info make_test(bool expected, bool expression, char *name) {
    struct test_info t = {.expected = expected, .expression = expression};
    if (strlen(name) < 49)
        memcpy(t.name, name, strlen(name));
    else
        memcpy(t.name, name, 49);
    return t;
}

/** runs all tests in every module, called in kmain() if testing is enabled */
void RUN_ALL_TESTS() {
    printf("NUMBER MODULES: %d\n", num_modules_made);
    uint32_t i;
    for (i = 0; i < NUM_MODULES && i < num_modules_made; i++) {
        test_module(modules[i]);
    }
}

/** runs the tests in the given test_module
 * 
 * @param module: module to run
 */
void test_module(struct test_module module) {
    int num_passed = 0;
    printf("TESTING MODULE: %s\n", module.name);

    int i;
    for (i = 0; i < TESTS_PER_MODULE && i < module.num_tests; i++)
        if (test(module.tests[i].expected, module.tests[i].expression, module.tests[i].name))
            num_passed++;
    
    if (module.num_tests >= TESTS_PER_MODULE)
        printf("TESTS DONE FOR MODULE: %s\nNUM_PASSED: %d\nNUM_FAILED: %d\n\n", module.name, num_passed, TESTS_PER_MODULE - num_passed);
    else
        printf("TESTS DONE FOR MODULE: %s\nNUM_PASSED: %d\nNUM_FAILED: %d\n\n", module.name, num_passed, module.num_tests - num_passed);
}

/** evaluates a test with the given inputs
 * 
 * @param expected: expected result of test
 * @param expression: expression to test
 * @param name: name of test
 * 
 * @return result of the test
 */
bool test(bool expected, bool expression, char *name) {
    bool result = expression == expected;

    if (test_prints) {
        if (result)
            printf("test %s passed: expected %B, got %B\n", name, expected, expression);
        else
            printf("test %s failed: expected %B, got %B\n", name, expected, expression);
    }

    return result;
}