/* Implementation of standard library IO functions.
 * NOTE: THIS IMPLEMENTATION ISN'T C99 COMPLIANT */

/* includes */
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stream.h>
#include "../drivers/display.h"
#include "../kernel/proc.h"
#include "../kernel/thread.h"

/* defines */

/* globals */

/* functions */

/** prints a null-terminated string to the screen
 * 
 * @param string: null-terminated string to print
 */
void kprint(char *string) {
    get_default_dis_driver()->dis_puts(string);
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    std_stream *out = GET_STDOUT(PROC_CUR());

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == 'd') {
                puts_std(out, int_to_string(va_arg(args, int)));
            } else if (*format == 'x') {
                puts_std(out, int_to_hexstring(va_arg(args, int)));
            } else if (*format == 's') {
                puts_std(out, (va_arg(args, char *)));
            } else if (*format == 'B') {
                bool arg = va_arg(args, int);
                if (arg)
                    puts_std(out, "true");
                else
                    puts_std(out, "false");
            }
        } else
            put_std(out, *format);

        format++;
    }

    va_end(args);
    return 0;
}

/** prints a formatted string to the screen
 * Format specifiers:
 *  - %d: decimal number
 *  - %x: hexadecimal number
 *  - %s: null-terminated string
 *  - %B: boolean value as a string
 * 
 * @param format: null-terminated string to format and print
 * @param ...: argument for each formatting specifier
 * 
 * @return 0 on success, -1 otherwise
 */
int kprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    display_t *dd = get_default_dis_driver();

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == 'd') {
                char *temp = int_to_string(va_arg(args, int));
                dd->dis_puts(temp);
            } else if (*format == 'x') {
                char *temp = int_to_hexstring(va_arg(args, int));
                dd->dis_puts(temp);
            } else if (*format == 's') {
                dd->dis_puts(va_arg(args, char *));
            } else if (*format == 'B') {
                bool arg = va_arg(args, int);
                if (arg)
                    dd->dis_puts("true");
                else
                    dd->dis_puts("false");
            }
        } else
            dd->dis_putc(*format);

        format++;
    }

    va_end(args);
    return 0;
}

/** composes a string with the same text that would be printed if format was used on printf, but instead of being printed, 
 * the content is stored as a C string in the buffer pointed by str. 
 * 
 * @param str: buffer to store formatted string in
 * @param format: string to format
 * @param ...: argument for each formatting specifier
 * 
 * @return number of characters written, excluding the null terminator
 */
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

/** prints a string with a newline to the screen 
 * 
 * @param string: null-terminated string to print
*/
void kprintln(char *string) {
    get_default_dis_driver()->dis_puts(string);
    get_default_dis_driver()->dis_putc('\n');
}