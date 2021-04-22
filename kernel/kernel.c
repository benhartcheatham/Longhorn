/* This file is the starting point for the kernel. It is responsible for initialization of 
 * most kernel subsystems. */

/* includes */
#include <stdbool.h>

/* libc */
#include <stdio.h>

/* Boot */
#include "../boot/multiboot.h"

/* Drivers */
#include "../drivers/vesa.h"
#include "../drivers/line.h"

/* Processes/Threads */
#include "proc.h"
#include "shell.h"

/* Memory Management */
#include "paging.h"

/* Interrupts */
#include "isr.h"

/* Testing */
#ifdef TESTS
    #include "test.h"
#endif

/* defines */

/* globals */
char *version_no = "0.3.0";

/** Main function of the kernel, starts the kernel and its subsystems. This function
 * returns to a tight loop and then is never scheduled again once completed.
 * 
 * @param mbi: mbi given by GRUB2
 * @param magic: unused
 */
void kmain(multiboot_info_t *mbi, unsigned int magic __attribute__ ((unused))) {
    init_idt();
    init_alloc(mbi);
    init_processes();

    display_init((void *) mbi);

    #ifndef TESTS
        shell_init();
    #endif

    #ifdef TESTS
        init_testing(true);
        RUN_ALL_TESTS(NULL);
    #endif

    #ifndef TESTS
        print_logo(HALF_LOGO);
        kprintf("\nWelcome to Longhorn!\nVersion no.: %s\nType <help> for a list of commands.\n> ", version_no);
    #endif
    
    enable_interrupts();

    //shouldn't run more than once
    while (1) {
        thread_yield();
    }
}
