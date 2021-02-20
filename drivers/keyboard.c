#include <stdint.h>
#include <string.h>
#include "keyboard.h"
#include "term.h"
#include "vesa.h"
#include "../kernel/isr.h"
#include "../kernel/port_io.h"

static const char kc_ascii[] = { 0, ASCII_ESCAPE, '1', '2', '3', '4', '5', '6',     
                          '7', '8', '9', '0', '-', '=', ASCII_BACKSPACE, ASCII_TAB, 
                          'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
                          'o', 'p', '[', ']', '\n', ASCII_LCTRL, 'a', 's', 
                          'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
                          '\'', '`', ASCII_SHIFT_PRESSED, '\\', 'z', 'x', 'c', 'v', 
                          'b', 'n', 'm', ',', '.', '/', ASCII_SHIFT_PRESSED, '*', '?', ' '};

static const char kc_ascii_cap[] = { 0, ASCII_ESCAPE, '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', ASCII_BACKSPACE, ASCII_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '[', ']', '\n', ASCII_LCTRL, 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ';', '\'', '`', ASCII_SHIFT_PRESSED, '\\', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', ',', '.', '/', ASCII_SHIFT_PRESSED, '*', '?', ' '};  // the remaining ? is the ALT key, don't know what to print there

static const char *escape_sequences[] = { "[nA", // move up n lines
                                          "[nB", // move down n lines
                                          "[nC", // move right n spaces
                                          "[nD", // move left n spaces
                                          "[m;nH", //move cursor to (m,n)
                                          "[sJ", "[sK", // clear screen, not fully implemented
                                          "[nL", // insert n lines
                                          "[nM", // delete n lines
                                          "[nP", // delete n chars
                                          "[n@", // insert n chars
                                          "[nm", // enable rendition
                                          "M"}; // scroll screen backward if cursor is at the top line

static int keyboard_set_mode(key_modes_t mode);
static int keyboard_set_output(key_driver_t *kd, term_t *t);

static int ischar(uint8_t c);
static int write_escape_sequence(term_t *t, const char *sequence, char param1, char param2);

key_driver_t default_kd;

/* keyboard interrupt handler
   writes characters pressed to the terminal */
void keyboard_handler(struct register_frame *r __attribute__ ((unused))) {
    uint8_t keycode = inb(0x60);

    if (default_kd.output == NULL)
        return;
    
    if (keycode == KC_LSHIFT || keycode == KC_RSHIFT|| keycode == KC_CAPS_LOCK || keycode == KC_LSHIFT_RELEASED || keycode == KC_RSHIFT_RELEASED) {
        default_kd.capitalize = !default_kd.capitalize;
        return;
    }

    if (KRAW) {
        if (default_kd.capitalize == true)
            default_kd.output->writec(default_kd.output, kc_ascii_cap[keycode]);
        else
            default_kd.output->writec(default_kd.output, kc_ascii[keycode]);
    } else {
        switch (keycode) {
            // need to figure out the keycodes for the arrow keys
            case KC_BACKSPACE:
                write_escape_sequence(default_kd.output, escape_sequences[9], 1, 0);
                break;
            case KC_TAB:
                write_escape_sequence(default_kd.output, escape_sequences[10], 5, 0);
                break;
            case KC_ENTER:
                default_kd.output->writec(default_kd.output, '\n');
                break;
            default:
                if (ischar(keycode)) {
                    if (default_kd.capitalize == true)
                        default_kd.output->writec(default_kd.output, kc_ascii_cap[keycode]);
                    else
                        default_kd.output->writec(default_kd.output, kc_ascii[keycode]);

                    return;
                }

                // if not supported just write out the keycode
                // char *hex_code = int_to_hexstring(keycode);
                // for (size_t i = 0; i < strlen(hex_code); i++)
                //     default_kd.output->writec(default_kd.output, hex_code[i]);
                break;    
        }
    }


}

static int ischar(uint8_t c) {
    if (c == KC_ESCAPE || c == KC_LCTRL || c == KC_LEFT_ALT)
        return false;
    return true;
}

static int write_escape_sequence(term_t *t, const char *sequence, char param1, char param2) {
    if (strlen(sequence) == 1) {
        t->writec(t, sequence[0]);
        return 0;
    }

    t->writec(t, sequence[0]);
    t->writec(t, param1);
    t->writec(t, sequence[2]);

    if (strlen(sequence) > 3) {
        t->writec(t, param2);
        t->writec(t, sequence[4]);
    }

    return 0;
}

static int keyboard_set_mode(key_modes_t mode) {
    default_kd.mode = mode;
    return 0;
}

static int keyboard_set_output(key_driver_t *kd, term_t *t) {
    kd->output = t;
    return 0;
}

/* initializes keyboard interrupt handler and key buffer 
   takes in a void * to a terminal struct */
void init_keyboard() {
    default_kd.init = NULL;
    default_kd.handler = keyboard_handler;
    default_kd.set_mode = keyboard_set_mode;
    default_kd.set_output = keyboard_set_output;

    default_kd.mode = KCOOKED;
    default_kd.output = NULL;
    default_kd.capitalize = false;
    default_kd.ctrl_pressed = false;
    default_kd.alt_pressed = false;

    register_interrupt_handler(IRQ01, default_kd.handler);
}

key_driver_t *get_default_kd() {
    return &default_kd;
}