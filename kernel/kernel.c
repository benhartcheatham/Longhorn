#include <stdbool.h>

/* Boot */
#include "../boot/multiboot.h"

/* Drivers */

/* libc */
#include "../libc/stdio.h"

/* Processes/Threads */
#include "proc.h"

/* Memory Management */
#include "kalloc.h"

/* Interrupts */
#include "isr.h"


/* testing functions */
void test(bool expected, bool expression, int num);
void test_procs();

void kmain(multiboot_info_t *mb, unsigned int magic __attribute__ ((unused))) {
    clear_screen();
    
    init_idt();
    init_alloc(mb);
    init_processes();
    test_procs();


    enable_interrupts();

}

void test(bool expected, bool expression, int num) {
    if (expression == expected)
        printf("TEST (%d) PASSED: EXPECTED %d, GOT: %d\n", num, expected, expression);
    else
        printf("TEST (%d) FAILED: EXPECTED %d, GOT: %d\n", num, expected, expression);
}

void test_procs() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        proc_create("test", NULL);
    }

    print_names();
}
