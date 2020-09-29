#include <stdint.h>
#include "vesa.h"
#include "vga_font.h"
#include "../boot/multiboot.h"
#include "../kernel/port_io.h"

static uint32_t *framebuffer_addr;
static uint32_t width;
static uint32_t height;
static uint32_t pitch;
static uint32_t bpp;
static uint8_t red_mask, green_mask, blue_mask;

static uint32_t num_rows;
static uint32_t num_cols;
static uint32_t cursor_x;
static uint32_t cursor_y;
static uint32_t current_x;
static uint32_t current_y;

static void scroll();

void init_vesa(multiboot_info_t *mbi) {
    framebuffer_addr = (uint32_t *) ((uint32_t) mbi->framebuffer_addr);
    width = mbi->framebuffer_width;
    height = mbi->framebuffer_height;
    pitch = mbi->framebuffer_pitch;
    bpp = mbi->framebuffer_bpp;
    //shutdown if not in 32-bit color mode
    if (bpp != 32 || mbi->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB)
        outw(0x604, 0x2000);

    red_mask = 0xff << mbi->framebuffer_red_field_position;
    green_mask = 0xff << mbi->framebuffer_green_field_position;
    blue_mask = 0xff << mbi->framebuffer_blue_field_position;

    num_cols = width / FONT_WIDTH;
    num_rows = height / FONT_HEIGHT;
    cursor_x = 0;
    cursor_y = 0;
    current_x = 0;
    current_y = 0;

}

void vesa_set_cursor(uint32_t x, uint32_t y) {
    if (y <= num_rows && x <= num_cols) {
        current_x = FONT_WIDTH * x;
        cursor_x = x;

        current_y = FONT_HEIGHT * y;
        cursor_y = y;
    }
}

void vesa_print_char(char c) {
    if (c == '\n') {
        vesa_set_cursor(0, cursor_y + 1);
        scroll();
        return;
    } else if (c == '\t') {
        if (cursor_x <= num_cols - 5)
            vesa_set_cursor(cursor_x + 5, cursor_y);
        else
            vesa_set_cursor(cursor_x + (num_cols - 5), cursor_y);
        return;
    }

    uint32_t *pixel_pos = framebuffer_addr + (current_y * width) + current_x;

    int i;
    for (i = 0; i < 16; i++) {
        char c_font = (char) vga_font[(c * 16) + i];

        int j;
        for (j = 0; j < 8; j++)
            if (c_font & (1 << j))
                pixel_pos[8 - j] = 0x00ffffff;
        
        pixel_pos += width;
    }

    current_x += FONT_WIDTH;
    cursor_x++;
    scroll();
}

void vesa_print_backspace() {
    vesa_set_cursor(--cursor_x, cursor_y);

    uint32_t *pixel_pos = framebuffer_addr + (current_y * width) + current_x;

    int i;
    for (i = 0; i < 16; i++) {
        char c_font = (char) vga_font[(' ' * 16) + i];

        int j;
        for (j = 0; j < 8; j++)
            if (c_font & (1 << j))
                pixel_pos[8 - j] = 0x00ffffff;
        
        pixel_pos += width;
    }

    scroll();
}

void vesa_print(char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vesa_print_char(string[i]);
}

void vesa_println(char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vesa_print_char(string[i]);
    vesa_print_char('\n');
}

void vesa_clear_screen() {
    uint32_t x;
    uint32_t y;
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            framebuffer_addr[(y * height) + x] = BACKGROUND;
    
    vesa_set_cursor(0, 0);
}

static void scroll() {
    if (cursor_y >= num_rows) {
        uint32_t x;
        uint32_t y;
        for (y = 0; y < height; y++)
            for (x = 0; x < width; x++)
                framebuffer_addr[(y * height) + x] = framebuffer_addr[(y * height + FONT_HEIGHT) + x];
    
        vesa_set_cursor(0, num_rows - 1);
    }
}


