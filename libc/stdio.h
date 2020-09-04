#ifndef _STDIO_H
#define _STDIO_H

/* includes */
#include <stdint.h>
#include "../drivers/vga.h"

/* defines */

/* structs */

/* typedefs */

/* functions */

/* functions for printing to the screen */
void print(char *string);
void println(char *string);
int printf(const char *string, ...);
int sprintf(const char *string, char *buffer, ...);
void print_align(char *string, uint16_t alignment);

/* shouldn't strictly be in this file 
   but are included for right now */
void clear_screen();
void copy_screen(char *buffer);
uint16_t get_cursor_offset();
void set_cursor_offset(uint16_t x, uint16_t y);
void set_color(enum vga_color fg, enum vga_color bg);
void set_fg_color(enum vga_color c);
void set_bg_color(enum vga_color c);
void set_default_color();
uint16_t get_color();

#endif

