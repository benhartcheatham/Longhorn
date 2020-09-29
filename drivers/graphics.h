#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include <stdint.h>
#include "../boot/multiboot.h"

#define GRAPHICS_MODE 0
#define TEXT_MODE 1

#define BLACK 0x00000000
#define WHITE 0x00ffffff
#define RED 0x00ff0000
#define GREEN 0x0000ff00
#define BLUE 0x000000ff

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

/* printing functions */
void graphics_print_char(char c);
void graphics_print(char *string);
void graphics_println(char *string);
void graphics_print_align(char *string, uint16_t alignment);
void graphics_print_backspace();

void init_graphics(multiboot_info_t *mbi);
uint8_t get_graphics_mode();

/* screen-wide functions */
void graphics_clear_screen();

//void graphics_copy_screen();

/* cursor functions */
uint32_t graphics_get_offset();
void graphics_set_cursor(uint32_t x, uint32_t y);

/* color functions */
void graphics_set_fg_color(uint32_t c);
void graphics_set_bg_color(uint32_t c);
void graphics_set_color(uint32_t fg, uint32_t bg);

#endif