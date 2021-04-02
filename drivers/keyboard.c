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
static char rel_buff[3];
static bool rel_flag = false;

/* functions */

/** keyboard interrupt handler
 * writes characters pressed to the screen 
 * 
 * @param r: unused
 */
static void keyboard_handler(struct register_frame *r __attribute__ ((unused))) {
    uint8_t scancode = inb(0x60);
    out_term = get_default_terminal();

    // if this is below KC_MAX i get another event going through for
    // release scancodes, need to fix
    if (scancode < SC_RELEASED)
        out_term->term_in(out_term, scancode);
    // else if (rel_flag) {
    //     rel_buff[1] = scancode;
    //     out_term->term_ins(out_term, rel_buff);
    //     rel_flag = false;
    // } else
    //     rel_flag = true;

}

/** initializes keyboard interrupt handler and key buffer */
void init_keyboard() {
    rel_buff[0] = KC_RELEASED;
    rel_buff[2] = 0;
    rel_flag = false;

    register_interrupt_handler(IRQ01, keyboard_handler);
}