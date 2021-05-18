/* Default implementation of the keyboard driver. */

/* includes */
#include <stdint.h>
#include <stdbool.h>
#include "../kernel/isr.h"
#include "../kernel/port_io.h"
#include "keyboard.h"
#include "terminal.h"

/* defines */

/* globals */
static term_t *out_term = NULL;

/* functions */

/** keyboard interrupt handler
 * writes characters pressed to the screen 
 * 
 * @param r: unused
 */
static void keyboard_handler(struct register_frame *r __attribute__ ((unused))) {
    uint8_t scancode = inb(0x60);
    if (out_term == NULL)
        out_term = get_default_terminal();

    //#ifdef SCANCODE_SET1
    if (scancode == SC_LALT_REL || scancode == SC_LSHIFT_REL || scancode == SC_LCTRL_REL 
        || scancode == SC_RSHIFT_REL) {
        out_term->term_in(out_term, scancode);
        return;
    }
    //#endif
    // if this is below KC_MAX i get another event going through for
    // release scancodes, need to fix
    if (scancode < SC_RELEASED)
        out_term->term_in(out_term, scancode);
    
}

/** initializes keyboard interrupt handler and key buffer */
void init_keyboard() {
    register_interrupt_handler(IRQ01, keyboard_handler);
}