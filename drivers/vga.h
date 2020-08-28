#include <stdint.h>

#ifndef vga_H
#define vga_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_COLS 80
#define MAX_ROWS 25
#define SCREEN_SIZE MAX_COLS*MAX_ROWS
//Attribute byte for default color scheme
#define WHITE_ON_BLACK 0x000f
#define DEFAULT_BACKGROUND WHITE_ON_BLACK

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

#define DEFAULT_FG VGA_COLOR_WHITE
#define DEFAULT_BG VGA_COLOR_BLACK

#endif

void vga_print_char(char c);
uint16_t get_offset();
void set_cursor(uint16_t x, uint16_t y);
void vga_print(char *string);
void vga_println(char *string);
void vga_print_align(char *string, uint16_t alignment);
void vga_clear_screen();
void vga_copy_screen(char *buffer);
void print_backspace();
void vga_set_fg_color(enum vga_color c);
void vga_set_bg_color(enum vga_color c);
void vga_set_color(enum vga_color fg, enum vga_color bg);
uint16_t vga_get_color();