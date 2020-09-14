#include <stdbool.h>
#include "test.h"
#include "../libc/stdio.h"

#define NUM_MODULES 10

static bool test_prints = false;
static bool num_modules_made = 0;
struct test_module modules[NUM_MODULES];

//put all tests in here
void init_testing(bool enable_test_prints) {
    test_prints = enable_test_prints;

    /* Below is a model of how a test module should be made */
    /*
        modules[0].name = "Test";
        modules[0].num_tests = 4;
        modules[0].tests[0].expected = bool;
        modules[0].tests[0].expression = bool expression;
        modules[0].tests[0].num = 1;

        modules[0].tests[1].expected = bool;
        modules[0].tests[1].expression = bool expression;
        modules[0].tests[1].num = 2;

        modules[0].tests[2].expected = bool;
        modules[0].tests[2].expression = bool expression;
        modules[0].tests[2].num = 3;

        modules[0].tests[3].expected = bool;
        modules[0].tests[3].expression = bool expression;
        modules[0].tests[3].num = 4;

        num_modules_made++;
    */
}

void RUN_ALL_TESTS() {
    int i;
    for (i = 0; i < NUM_MODULES && i < num_modules_made; i++) {
        test_module(modules[i]);
    }
}

void test_module(struct test_module module) {
    int num_passed = 0;
    printf("TESTING MODULE: %s\n", module.name);
    if (module.num_tests > TESTS_PER_MODULE) 
        printf("WARNING: MODULE %s HAS MORE THAN MAXIMUM NUMBER OF TESTS PER MODULE. ONLY RUNNING THE FIRST %d TESTS...\n", module.name, module.num_tests);

    int i;
    for (i = 0; i < TESTS_PER_MODULE && i < module.num_tests; i++)
        if (test(module.tests[i].expected, module.tests[i].expression, module.tests[i].num))
            num_passed++;
    
    if (module.num_tests >= TESTS_PER_MODULE)
        printf("TESTS DONE FOR MODULE: %s\nNUM_PASSED: %d\nNUM_FAILED: %d\n\n", module.name, num_passed, TESTS_PER_MODULE - num_passed);
    else
        printf("TESTS DONE FOR MODULE: %s\nNUM_PASSED: %d\nNUM_FAILED: %d\n\n", module.name, num_passed, module.num_tests - num_passed);
}

bool test(bool expected, bool expression, int num) {
    bool result = expression == expected;

    if (test_prints) {
        if (result)
            printf("test (%d) passed: expected %d, got: %d\n", num, expected, expression);
        else
            printf("test (%d) failed: expected %d, got: %d\n", num, expected, expression);
    }

    return result;
}