#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include "string.h"
#include "../drivers/vga.h"

/* prints a string to the screen */
void print(char *string) {
    vga_print(string);
}

/* prints a formatted string to the screen */
int printf(const char *string, ...) {
    const char *curr = string;
    
    va_list args;
    va_start(args, string);

    while (*curr != '\0') {
        if (*curr == '%') {
            curr++;
            if (*curr == 'd') {
                char *temp = int_to_string(va_arg(args, int));
                print(temp);
            } else if (*curr == 'x') {
                char *temp = int_to_hexstring(va_arg(args, int));
                print(temp);
            } else if (*curr == 's')
                print(va_arg(args, char *));
        } else
            vga_print_char(*curr);

        curr++;
    }

    va_end(args);
    return 0;
}

/* THIS IS CURRENTLY UNIMPLEMENTED. DO NOT USE */

int sprintf(const char *string __attribute__ ((unused)), char *buffer __attribute__ ((unused)), ...) {
    //TODO: Implement this function
    return -1;
}

/* prints a string with a newline to the screen */
void println(char *string) {
    vga_println(string);
}

/* prints an aligned string to the screen*/
void print_align(char *string, uint16_t alignment) {
    vga_print_align(string, alignment);
}

/* clears the screen */
void clear_screen() {
    vga_clear_screen();
}

/* copies the screen into buffer */
void copy_screen(char *buffer) {
    vga_copy_screen(buffer);
}

/* gets the cursor postion */
uint16_t get_cursor_offset() {
    return get_offset();
}

/* sets the cursor position */
void set_cursor_offset(uint16_t x, uint16_t y) {
    set_cursor(x,y);
}

/* sets the color of text */
void set_color(enum vga_color fg, enum vga_color bg) {
    vga_set_color(fg, bg);
}

/* sets the foreground color of text */
void set_fg_color(enum vga_color c) {
    vga_set_fg_color(c);
}

/*sets the background color of text */
void set_bg_color(enum vga_color c) {
    vga_set_bg_color(c);
}

void set_default_color() {
    vga_set_color(DEFAULT_FG, DEFAULT_BG);
}

/* gets the color of text
   background is in the first 4 bits, foreground in last 4 bits */
uint16_t get_color() {
    return vga_get_color();
}