#ifndef _VESA_H
#define _VESA_H

#include <stdint.h>
#include "graphics.h"
#include "../boot/multiboot.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

void init_vesa(multiboot_info_t *mbi);
void vesa_set_cursor(uint32_t x, uint32_t y);
void vesa_print_char(char c);
void vesa_print_backspace();
void vesa_print(char *string);
void vesa_println(char *string);
void vesa_print_align(char *string, uint16_t alignment);
void vesa_clear_screen();
void vesa_set_color(uint32_t fg, uint32_t bg);
void vesa_set_fg_color(uint32_t fg);
void vesa_set_bg_color(uint32_t bg);
void vesa_set_default_color();

#endif