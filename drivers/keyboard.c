#include <stdint.h>
#include "keyboard.h"
#include "vga.h"
#include "../kernel/isr.h"
#include "../kernel/proc.h"
#include "../kernel/port_io.h"

static int capitalize = -1;
static struct process *active;

const char kc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
                          '7', '8', '9', '0', '-', '=', '?', '?', 
                          'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
                          'o', 'p', '[', ']', '?', '?', 'a', 's', 
                          'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
                          '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
                          'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

const char kc_ascii_cap[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

/* keyboard interrupt handler
   writes characters pressed to the screen */
static void keyboard_handler(struct register_frame *r __attribute__ ((unused))) {
    uint8_t keycode = inb(0x60);

    if (proc_get_active() != active)
        active = proc_get_active();
    
    if (keycode == ENTER) {
        put_std(&active->stdin, '\n');

    } else if (keycode == BACKSPACE) {
        put_std(&active->stdin, '\b');

    } else if (keycode == SHIFT_PRESSED || keycode == SHIFT_RELEASED || keycode == CAPS_LOCK_PRESSED) {
        capitalize *= -1;

    } else if (keycode <= KC_MAX && keycode > 0) {
        if (capitalize == -1)
            put_std(&active->stdin, kc_ascii[keycode]);
        else
            put_std(&active->stdin, kc_ascii_cap[keycode]);
    }
}

/* initializes keyboard interrupt handler and key buffer */
void init_keyboard() {
    register_interrupt_handler(IRQ01, keyboard_handler);
}