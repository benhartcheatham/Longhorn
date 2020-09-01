/* Drivers */
#include "../drivers/vga.h"

/* Interrupts */
#include "isr.h"

void kmain() {
    vga_clear_screen();
    vga_println("Hello, World!");
    
    init_irqs();
    //enable_interrupts();
    //asm volatile("int $0");
    for(;;) {
        asm volatile("hlt");
    }
}