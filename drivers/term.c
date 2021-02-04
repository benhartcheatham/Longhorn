/* Terminal is only meant for input from the keyboard currently,
 * this should be expanded to be any hardware device later on.
 * There can also only be one terminal on the machine at once,
 * this should probably be changed. */

#include "term.h"
#include "vesa.h"
#include "vga.h"
#include <stdio.h>

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

static int8_t capitalize = 0;

static term_t default_term;

static int is_whitespace(uint8_t keycode);

term_t terminal_init(std_stream *in, std_stream *out, key_driver_t *kd, dis_driver_t *dd, void *aux) {
    term_t temp;
    temp.terminal_init = terminal_init;
    temp.terminal_din = terminal_in;
    temp.terminal_dout = terminal_out;
    temp.terminal_mode = terminal_mode;
    temp.terminal_putc = terminal_putc;

    temp.kd = kd;
    temp.dd = dd;
    temp.terminal_din(&temp, in);
    temp.terminal_dout(&temp, out);
    temp.terminal_mode(&temp, KEY_COOKED);

    return temp;
}

int terminal_mode(term_t *t, key_modes_t mode) {
    if (t) {
        t->mode = mode;
        t->kd->keyboard_mode(mode);
    } else
        return -1;
    
    return 0;
}

int terminal_in(term_t *t, std_stream *in) {
    if (t)
        t->in = in;
    else
        return -1;
    
    return 0;
}

int terminal_out(term_t *t, std_stream *out) {
    if (t)
        t->out = out;
    else
        return -1;
    return 0;
}


/* puts the char into the stdin of out and sends the char to the 
   configured driver */
int terminal_putc(uint8_t c) {
    
    return 0;
}



static int is_whitespace(uint8_t keycode) {
    if (keycode == BACKSPACE || keycode == SHIFT_PRESSED || 
            keycode == SHIFT_RELEASED || keycode == CAPS_LOCK_PRESSED || keycode == ENTER)
        return 1;
    return 0;
}