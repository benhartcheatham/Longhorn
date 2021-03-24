#include <stdint.h>
#include <stdbool.h>
#include "keyboard.h"
#include "terminal.h"
#include "../kernel/isr.h"
#include "../kernel/port_io.h"

static term_t *out_term = NULL;
static char rel_buff[3];
static bool rel_flag;

/* keyboard interrupt handler
   writes characters pressed to the screen */
static void keyboard_handler(struct register_frame *r __attribute__ ((unused))) {
    uint8_t scancode = inb(0x60);
    out_term = get_default_terminal();
    if (scancode != SC_RELEASED && !rel_flag)
        out_term->term_in(out_term, scancode);
    else if (rel_flag) {
        rel_buff[1] = scancode;
        out_term->term_ins(out_term, rel_buff);
        rel_flag = false;
    } else
        rel_flag = true;

}

/* initializes keyboard interrupt handler and key buffer */
void init_keyboard() {
    rel_buff[0] = KC_RELEASED;
    rel_buff[2] = 0;
    rel_flag = false;

    register_interrupt_handler(IRQ01, keyboard_handler);
}