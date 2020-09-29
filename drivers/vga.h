#ifndef vga_H
#define vga_H

/* includes */
#include <stdint.h>
#include "graphics.h"

/* defines */
#define VIDEO_ADDRESS 0xb8000
#define MAX_COLS 80
#define MAX_ROWS 25
#define SCREEN_SIZE MAX_COLS*MAX_ROWS
//Attribute byte for default color scheme
#define WHITE_ON_BLACK 0x000f
#define DEFAULT_BACKGROUND WHITE_ON_BLACK
#define DEFAULT_FG VGA_COLOR_WHITE
#define DEFAULT_BG VGA_COLOR_BLACK

/* structs */

/* typedefs */

/* functions */

/* printing functions */
void vga_print_char(char c);
uint16_t get_offset();
void vga_set_cursor(uint16_t x, uint16_t y);
void vga_print(char *string);
void vga_println(char *string);
void vga_print_align(char *string, uint16_t alignment);
void vga_clear_screen();
void vga_copy_screen(char *buffer);
void vga_print_backspace();

/* color functions */
void vga_set_fg_color(enum vga_color c);
void vga_set_bg_color(enum vga_color c);
void vga_set_color(enum vga_color fg, enum vga_color bg);
uint16_t vga_get_color();

#endif