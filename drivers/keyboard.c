#include <stdint.h>
#include "keyboard.h"
#include "term.h"
#include "../kernel/isr.h"
#include "../kernel/port_io.h"

static const char kc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
                          '7', '8', '9', '0', '-', '=', '?', '?', 
                          'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
                          'o', 'p', '[', ']', '\n', '?', 'a', 's', 
                          'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
                          '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
                          'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

static const char kc_ascii_cap[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '[', ']', '\n', '?', 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};


term_t *output_terminal = NULL;
key_driver_t default_kd;

/* keyboard interrupt handler
   writes characters pressed to the screen */
int keyboard_handler(struct register_frame *r __attribute__ ((unused))) {
    uint8_t keycode = inb(0x60);
    
    output_terminal->terminal_putc(keycode);
    return 0;
}

int keyboard_direct(std_stream *in) {
    default_kd.in = in;
    return 0;
}

int keyboard_set_mode(key_modes_t mode) {
    default_kd.mode = mode;
    return 0;
}

/* initializes keyboard interrupt handler and key buffer 
   takes in a void * to a terminal struct */
void init_keyboard(void *aux) {
    default_kd.keyboard_handler = keyboard_handler;
    default_kd.keyboard_init = NULL;
    default_kd.keyboard_direct = keyboard_direct;
    default_kd.keyboard_mode = keyboard_set_mode;
    
    output_terminal = (term_t *) aux;
    default_kd.keyboard_direct(output_terminal->in);

    register_interrupt_handler(IRQ01, keyboard_handler);
}