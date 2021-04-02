/* Defines the functionality of the VGA driver. Currrently deprecated in favor of the VESA driver. */
#ifndef VGA_H
#define VGA_H

/* includes */
#include <stdint.h>

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

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_PURPLE = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_GRAY = 7,
	VGA_COLOR_DARK_GRAY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_PURPLE = 13,
	VGA_COLOR_YELLOW = 14,
	VGA_COLOR_WHITE = 15,
};
/* typedefs */

/* functions */

/* printing functions */
void vga_print_char(char c);
void vga_print(char *string);
void vga_println(char *string);
void vga_print_align(char *string, uint16_t alignment);
void vga_clear_screen();
void vga_copy_screen(char *buffer);
void vga_print_backspace();

/* cursor functions */
uint16_t get_offset();
void vga_set_cursor(uint16_t x, uint16_t y);
void vga_show_cursor();
void vga_hide_cursor();

/* color functions */
void vga_set_fg_color(enum vga_color c);
void vga_set_bg_color(enum vga_color c);
void vga_set_color(enum vga_color fg, enum vga_color bg);
uint16_t vga_get_color();

#endif