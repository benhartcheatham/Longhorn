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

/* Standard Streams */
static std_stream STDIN;
static std_stream STDOUT;
static std_stream STDERR;

/* testing functions */
void test(bool expected, bool expression, int num);
void dummy(void *aux);

void kmain(multiboot_info_t *mb, unsigned int magic __attribute__ ((unused))) {
    stdin = init_std(&STDIN);
    stdout = init_std(&STDOUT);
    stderr = init_std(&STDERR);
    clear_screen();
    
    init_idt();
    init_alloc(mb);
    init_processes();
    
    enable_interrupts();

}

void test(bool expected, bool expression, int num) {
    if (expression == expected)
        printf("TEST (%d) PASSED: EXPECTED %d, GOT: %d\n", num, expected, expression);
    else
        printf("TEST (%d) FAILED: EXPECTED %d, GOT: %d\n", num, expected, expression);
}

void dummy(void *aux __attribute__ ((unused))) {
    print("dummy\n");
    return;
}
