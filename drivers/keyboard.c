#include <stdint.h>
#include "keyboard.h"
#include "vga.h"
#include "../kernel/isr.h"
#include "../kernel/port_io.h"
#include "../libc/stdio.h"

char *keyboard_get_key_buffer();
void keyboard_flush_key_buffer();
static void append_to_buffer(char c);
static void shrink_buffer(int size);

static char key_buffer[TERMINAL_LIMIT + 1];     //size is limit+1 to allow for a null-terminator
static int buffer_index = 0;
static int capitalize = -1;

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

    if (keycode == ENTER) {
        vga_print("\n");
        keyboard_flush_key_buffer();

    } else if (keycode == BACKSPACE) {

        if (buffer_index > 0) {
            shrink_buffer(1);
            print_backspace();
        }

    } else if (keycode == SHIFT_PRESSED || keycode == SHIFT_RELEASED || keycode == CAPS_LOCK_PRESSED) {
        capitalize *= -1;

    } else if (keycode <= KC_MAX && keycode > 0) {
        if (capitalize == -1) {
            append_to_buffer(kc_ascii[keycode]);
            vga_print_char(kc_ascii[keycode]);
        } else {
            append_to_buffer(kc_ascii_cap[keycode]);
            vga_print_char(kc_ascii_cap[keycode]);
        }
        
    }
}

/* initializes keyboard interrupt handler and key buffer */
void init_keyboard() {
    register_interrupt_handler(IRQ01, keyboard_handler);
    keyboard_flush_key_buffer();
}

/* adds char c to the key buffer */
static void append_to_buffer(char c) {
    if (buffer_index < TERMINAL_LIMIT)
        key_buffer[buffer_index++] = c;
}

/* deletes the last size characters from the buffer
   size must be less than or equal to the current index of the buffer and greater
   than 0 */
static void shrink_buffer(int size) {
    if (size <= buffer_index && buffer_index > 0)
        key_buffer[--buffer_index] = '\0';

    if (size == 1 && buffer_index == 0)
        key_buffer[buffer_index] = '\0';
}

/* flushes/clears the key buffer */
void keyboard_flush_key_buffer() {
    buffer_index = 0;

    int i;
    for (i = 0; i < TERMINAL_LIMIT + 1; i++)
        key_buffer[i] = '\0';
}

/* returns the address to the key buffer */
char *keyboard_get_key_buffer() {
    return key_buffer;
}