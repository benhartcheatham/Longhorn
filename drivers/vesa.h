#ifndef _VESA_H
#define _VESA_H

#include <stdint.h>
#include "../boot/multiboot.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define WHITE 0xffffff
#define BLACK 0x000000
#define BACKGROUND BLACK
#define FOREGROUND WHITE

void init_vesa(multiboot_info_t *mbi);
void vesa_set_cursor(uint32_t x, uint32_t y);
void vesa_print_char(char c);
void vesa_print_backspace();
void vesa_print(char *string);

#endif