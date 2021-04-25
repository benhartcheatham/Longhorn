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
#include "palloc.h"
#include "proc.h"
#include "thread.h"
#include "port_io.h"
#include "paging.h"

/* defines */
#define NUM_MODULES 10

/* globals */
static bool test_prints = false;
static uint32_t num_modules_made = 0;
struct test_module modules[NUM_MODULES];
extern struct process *init;
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
    kprintf("initializing tests\n");
    slab_alloc_t *slab_allocator = get_default_slab_allocator();
    void *slab_alloc_mem = NULL;
    void *slab_ret = NULL;
    void *slab_ret2 = NULL;

    kprintf("making kalloc tests\n");
    struct test_module kalloc = make_module("Kalloc");
    // #ifdef TESTS
    // these numbers are outdated, and it's kinda a stupid test
    // add_test(&kalloc, make_test(true, num_allocated() == (map_size() / PG_SIZE + 8), "Initialization"));
    // #endif
    add_test(&kalloc, make_test(true, (slab_alloc_mem = palloc()) != NULL, "Allocate mem for slab allocator"));
    add_module(&kalloc);

    kprintf("making slab alloc tests\n");
    struct test_module slab_alloc = make_module("Slab Alloc");
    kprintf("hello %x\n", slab_allocator->alloc);
    slab_ret = slab_allocator->alloc(slab_allocator, 1);
    kprintf("thought so\n");
    add_test(&slab_alloc, make_test(true, slab_ret != NULL, "Slab allocator allocation 1"));
    slab_ret2 = slab_allocator->alloc(slab_allocator, 30);
    kprintf("unless?\n");
    add_test(&slab_alloc, make_test(true, slab_ret2 != NULL, "Slab allocator allocation 2"));
    add_test(&slab_alloc, make_test(true, slab_allocator->free(slab_allocator, slab_ret, 1) == SLAB_SUCC, "Slab allocator free 1"));
    add_test(&slab_alloc, make_test(true, slab_allocator->free(slab_allocator, slab_ret2, 30) == SLAB_SUCC, "Slab allocator free 2"));
    kprintf("jk\n");
    add_module(&slab_alloc);

    // #ifdef TESTS
    // slab_print_list(get_default_slab_allocator());
    // #endif

    kprintf("making process tests\n");
    struct test_module procs = make_module("Processes");
    strcpy(procs.name, "Processes");    // this shouldn't have to happen, but i'm redoing this all soon anyway
    struct process *p1 = proc_create("test", proc_test_func, NULL);
    add_test(&procs, make_test(true, p1->pid == 1, "Create1"));

    const list_node_t *proc_node = proc_peek_all_list();
    while (proc_node != NULL && LIST_ENTRY(proc_node, struct process, node)->pid != p1->pid)
        proc_node = proc_node->next;

    int ret = 0;
    struct process *proc_handle = LIST_ENTRY(proc_node, struct process, node);
    add_test(&procs, make_test(true, proc_handle->num_live_threads == 1, "Create2"));
    proc_kill(proc_handle, &ret);

    add_test(&procs, make_test(true, ret == 0, "Kill"));
    add_module(&procs);

//     struct test_module paging = make_module("Paging");
//     //init_paging(&init->pgdir);
//     page_dir_t *fake_dir = init->pgdir;
//     vaddr_t vaddr = 0x102000;
//     pde_t *fake_pde = &fake_dir->tables[(vaddr >> 22) & 0x3FF];
//     add_test(&paging, make_test(true, fake_pde->present & !fake_pde->user & fake_pde->rw, "pde set correctly"));
//     // paging_kvmap(fake_dir, vaddr, vaddr);
//     kprintf("sizeof page_dir_t: %d\n", sizeof(page_dir_t));

//     page_table_t *fake_table = paging_traverse_pgdir(fake_dir, vaddr, false, 0);
//     pte_t *fake_pte = paging_traverse_pgtable(fake_table, vaddr, false, 0);
//     kprintf("fake_dir: %x\n", fake_dir);
//     add_test(&paging, make_test(true, fake_dir != NULL, "correct dir location"));
//     add_test(&paging, make_test(true, fake_table != NULL, "Page table allocated"));
//     add_test(&paging, make_test(true, fake_dir->tables[0].present, "pde present"));
//     add_test(&paging, make_test(true, fake_pte != NULL, "pte allocated"));
//     add_test(&paging, make_test(true, fake_pte == &fake_table->entries[258], "pte in correct location"));
//     add_test(&paging, make_test(true, fake_pte->present, "pte present"));
//     fake_pde = &fake_dir->tables[1];
//     kprintf("fake_dir->tables[1]: %x\n", *fake_pde);
//     add_test(&paging, make_test(false, fake_pde->present, "over allocation"));
//     add_test(&paging, make_test(true, get_current_pgdir() == fake_dir, "get page dir"));
//     add_module(&paging);
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
void RUN_ALL_TESTS(void *aux __attribute__ ((unused))) {
    kprintf("NUMBER MODULES: %d\n", num_modules_made);
    uint32_t i;
    for (i = 0; i < NUM_MODULES && i < num_modules_made; i++) {
        test_module(&modules[i]);
    }

    kprintf("exiting...\n");
    asm volatile("cli \n\t\
                  hlt");
    outw(0x604, 0x2000);    // issues shutdown command to QEMU
}

/** runs the tests in the given test_module
 * 
 * @param module: module to run
 */
void test_module(struct test_module *module) {
    int num_passed = 0;
    kprintf("TESTING MODULE: %s\n", module->name);

    int i;
    for (i = 0; i < TESTS_PER_MODULE && i < module->num_tests; i++)
        if (test(module->tests[i].expected, module->tests[i].expression, module->tests[i].name))
            num_passed++;

    if (module->num_tests >= TESTS_PER_MODULE)
        kprintf("TESTS DONE FOR MODULE: %s\nNUM_PASSED: %d\nNUM_FAILED: %d\n\n", module->name, num_passed, TESTS_PER_MODULE - num_passed);
    else
        kprintf("TESTS DONE FOR MODULE: %s\nNUM_PASSED: %d\nNUM_FAILED: %d\n\n", module->name, num_passed, module->num_tests - num_passed);
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
            kprintf("test %s passed: expected %B, got %B\n", name, expected, expression);
        else
            kprintf("test %s failed: expected %B, got %B\n", name, expected, expression);
    }

    return result;
}