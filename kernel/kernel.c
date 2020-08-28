#include "../drivers/vga.h"

void kmain() {
    vga_clear_screen();
    vga_println("Hello, World!");
    
    for(;;) {
        asm volatile("cli");
        asm volatile("hlt");
    }
}