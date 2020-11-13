#include <stdint.h>
#include "keyboard.h"
#include "terminal.h"
#include "../kernel/isr.h"
#include "../kernel/port_io.h"

/* keyboard interrupt handler
   writes characters pressed to the screen */
static void keyboard_handler(struct register_frame *r __attribute__ ((unused))) {
    uint8_t keycode = inb(0x60);
    
    terminal_put(keycode);
}

/* initializes keyboard interrupt handler and key buffer */
void init_keyboard() {
    register_interrupt_handler(IRQ01, keyboard_handler);
}