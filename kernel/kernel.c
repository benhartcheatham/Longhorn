/* Drivers */

/* libc */
#include "../libc/stdio.h"

/* Processes/Threads */
#include "proc.h"

/* Interrupts */
#include "isr.h"

void kmain() {
    clear_screen();
    println("Hello, World!");
    
    init_idt();
    init_processes();
    enable_interrupts();

}

