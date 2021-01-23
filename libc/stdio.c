/* THIS IMPLEMENTATION ISN'T C99 COMPLIANT */

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "stdio.h"
#include "string.h"
#include "../drivers/vesa.h"

/* prints a string to the screen */
void print(char *string) {
    vesa_print(string);
}

/* prints a formatted string to the screen */
int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == 'd') {
                char *temp = int_to_string(va_arg(args, int));
                vesa_print(temp);
            } else if (*format == 'x') {
                char *temp = int_to_hexstring(va_arg(args, int));
                vesa_print(temp);
            } else if (*format == 's') {
                vesa_print(va_arg(args, char *));
            } else if (*format == 'B') {
                bool arg = va_arg(args, int);
                if (arg)
                    vesa_print("true");
                else
                    vesa_print("false");
            }
        } else
            vesa_print_char(*format);

        format++;
    }

    va_end(args);
    return 0;
}

/* composes a string with the same text that would be printed if format was used on printf, but instead of being printed, 
   the content is stored as a C string in the buffer pointed by str. 
   returns the number of characters written, excluding the null terminator */
int sprintf(char *str, const char *format, ...) {
    int chars_written = 0;
    va_list args;
    va_start(args, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            const char *temp = NULL;

            if (*format == 'd')
                temp = int_to_string(va_arg(args, int));
            else if (*format == 'x')
                temp = int_to_hexstring(va_arg(args, int));
            else if (*format == 's')
                temp = va_arg(args, char *);
            else if (*format == 'B') 
                temp = va_arg(args, bool) ? "true" : "false";
            else {
                format++;
                continue;
            }

            int len = strlen(temp);
            strncpy(str, temp, len);
            str += len;
            chars_written += len;
        } else {
            *str = *format;
            str++;
            chars_written++;
        }

        format++;
    }

    *str = '\0';

    return chars_written;
}

/* prints a string with a newline to the screen */
void println(char *string) {
    vesa_println(string);
}

/* prints an aligned string to the screen*/
void print_align(char *string, uint16_t alignment) {
    vesa_print_align(string, alignment);
}

/* clears the screen */
void clear_screen() {
    vesa_clear_screen();
}

/* sets the cursor position */
void set_cursor_offset(uint16_t x, uint16_t y) {
    vesa_set_cursor(x,y);
}

/* sets the color of text */
void set_color(enum vga_color fg, enum vga_color bg) {
    vesa_set_color(fg, bg);
}

/* sets the foreground color of text */
void set_fg_color(enum vga_color c) {
    vesa_set_fg_color(c);
}

/*sets the background color of text */
void set_bg_color(enum vga_color c) {
    vesa_set_bg_color(c);
}

void set_default_color() {
    vesa_set_color(WHITE, BLACK);
}