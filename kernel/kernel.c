#include <stdbool.h>

/* Boot */
#include "../boot/multiboot.h"

/* Drivers */

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
#ifdef _TEST_H
    #include "test.h"
#endif


void kmain(multiboot_info_t *mb, unsigned int magic __attribute__ ((unused))) {
    clear_screen();
    
    init_idt();
    init_alloc(mb);
    init_processes();
    terminal_init();


    enable_interrupts();

}
