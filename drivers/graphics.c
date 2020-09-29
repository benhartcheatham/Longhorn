#include "graphics.h"
#include "vga.h"
#include "vesa.h"

#define VGA_DRIVER 0
#define VESA_DRIVER 1

uint8_t graphics_mode;

static void (*pri_c) (char);
static void (*pri) (char *);
static void (*pri_ln) (char *);
static void (*pri_align) (char *, uint16_t);
static void (*pri_backspace) (void);
static void (*clear) (void);
static void (*set_cur) (uint32_t, uint32_t);
static void (*show_cur) (void);
static void (*hide_cur) (void);
static void (*set_fg) (uint32_t);
static void (*set_bg) (uint32_t);
static void (*set_c) (uint32_t, uint32_t);

static void set_driver(uint8_t driver_no, multiboot_info_t *mbi);

void init_graphics(multiboot_info_t *mbi) {
    if (mbi->vbe_mode != 3)
        set_driver(VESA_DRIVER, mbi);
    else
        set_driver(VGA_DRIVER, mbi);
}

static void set_driver(uint8_t driver_no, multiboot_info_t *mbi) {
    if (driver_no == VESA_DRIVER) {
        init_vesa(mbi);
        pri = vesa_print;
        pri_c = vesa_print_char;
        pri_ln = vesa_println;
        pri_align = vesa_print_align;
        pri_backspace = vesa_print_backspace;
        clear = vesa_clear_screen;
        set_cur = vesa_set_cursor;
        set_fg = vesa_set_fg_color;
        set_bg = vesa_set_bg_color;
        set_c = vesa_set_color;
        show_cur = vesa_show_cursor;
        hide_cur = vesa_hide_cursor;
        graphics_mode = GRAPHICS_MODE;

    } else if (driver_no == VGA_DRIVER) {
        pri = vga_print;
        pri_c = vga_print_char;
        pri_ln = vga_println;
        pri_align = vga_print_align;
        pri_backspace = vga_print_backspace;
        clear = vga_clear_screen;
        set_cur = (void (*) (uint32_t, uint32_t)) vga_set_cursor;
        show_cur = vga_show_cursor;
        hide_cur = vga_hide_cursor;
        set_fg = (void (*) (uint32_t)) vga_set_fg_color;
        set_bg = (void (*) (uint32_t)) vga_set_bg_color;
        set_c = (void (*) (uint32_t, uint32_t)) vga_set_color;
        
        graphics_mode = TEXT_MODE;
    }
}

/* printing functions */

void graphics_print_char(char c) {
    pri_c(c);
}

void graphics_print(char *string) {
    pri(string);
}
void graphics_println(char *string) {
    pri_ln(string);
}

void graphics_print_align(char *string, uint16_t alignment) {
    pri_align(string, alignment);
}

void graphics_print_backspace() {
    pri_backspace();
}

/* screen-wide functions */

void graphics_clear_screen() {
    clear();
}

/* cursor functions */

void graphics_set_cursor(uint32_t x, uint32_t y) {
    set_cur(x, y);
}

void graphics_show_cursor() {
    show_cur();
}

void graphics_hide_cursor() {
    hide_cur();
}

/* color functions */
void graphics_set_fg_color(uint32_t c) {
    set_fg(c);
}

void graphics_set_bg_color(uint32_t c) {
    set_bg(c);
}

void graphics_set_color(uint32_t fg, uint32_t bg) {
    set_c(fg, bg);
}

uint8_t get_graphics_mode() {
    return graphics_mode;
}