#include <stdbool.h>

/* Boot */
#include "../boot/multiboot.h"

/* Drivers */
#include "../drivers/vesa.h"
#include "../drivers/line.h"

/* libc */
#include <stdio.h>

/* Processes/Threads */
#include "proc.h"
#include "shell.h"

/* Memory Management */
#include "kalloc.h"

/* Interrupts */
#include "isr.h"

/* Testing */
#ifdef TESTS
    #include "test.h"
#endif

/* Kernel data */
char *version_no = "0.3.0";

void kmain(multiboot_info_t *mbi, unsigned int magic __attribute__ ((unused))) {
    init_idt();
    init_alloc(mbi);
    init_processes();

    init_display((void *) mbi);

    shell_init();
    #ifdef TESTS
        init_testing(true);
        RUN_ALL_TESTS();
    #endif

    #ifndef TESTS
        print_logo(HALF_LOGO);
        printf("\nWelcome to Longhorn!\nVersion no.: %s\nType <help> for a list of commands.\n> ", version_no);
    #endif

    enable_interrupts();
    //shouldn't get to this point again
    
    // while (1) {
    //     thread_yield();
    // }
}
