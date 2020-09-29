#include <stdbool.h>

/* Boot */
#include "../boot/multiboot.h"

/* Drivers */
#include "../drivers/graphics.h"

/* libc */
#include "../libc/stdio.h"

/* Processes/Threads */
#include "proc.h"
#include "terminal.h"

/* Memory Management */
#include "kalloc.h"

/* Interrupts */
#include "isr.h"

/* Testing */
#ifdef TESTS
    #include "test.h"
#endif

/* Kernel data */
char *major_version_no = "0";
char *minor_version_no = "1";
char *build_version_no = "x";

void kmain(multiboot_info_t *mbi, unsigned int magic __attribute__ ((unused))) {
    init_graphics(mbi);
    init_idt();
    init_alloc(mbi);
    init_processes();
    terminal_init();

    #ifdef TESTS
        init_testing(true);
        RUN_ALL_TESTS();
    #endif

    #ifndef TESTS
        print_logo(HALF_LOGO);
        printf("\nWelcome to Longhorn!\nVersion no.: %s.%s.%s\nType <help> for a list of commands.\n> ", major_version_no, minor_version_no, build_version_no);
    #endif
    enable_interrupts();

}
