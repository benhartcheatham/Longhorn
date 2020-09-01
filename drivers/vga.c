#include <stdint.h>
#include "vga.h"
#include "../kernel/port_io.h"
#include "../libc/mem.h"

void scroll();

static uint16_t cursor_x;
static uint16_t cursor_y;
static uint16_t color = DEFAULT_BACKGROUND;

/* sets the cursor to x,y on the screen */
void set_cursor(uint16_t x, uint16_t y) {

    if (y <= MAX_ROWS && x <= MAX_COLS) {
        uint16_t location = y * 80 + x;
        outb(0x3D4, 14);
        outb(0x3D5, location >> 8);
        outb(0x3D4, 15);
        outb(0x3D5, location);

        cursor_x = x;
        cursor_y = y;
    }
}

/* gets the offest of the cursor */
uint16_t get_offset() {
    uint16_t pos = 0;
    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((uint16_t) inb(0x3D5)) << 8;
    return pos;
}

/* prints char c to the current cursor position */
void vga_print_char(char c) {
    if (c == '\n') {
        cursor_y++;
        cursor_x = 0;
        set_cursor(cursor_x, cursor_y);
        scroll();
        return;
    }

    uint16_t offset = get_offset();
    uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

    videomem += offset;
    *videomem = (color << 8) | c;

    cursor_x++;
    set_cursor(cursor_x, cursor_y);
    scroll();
}

/* removes the char before the current cursor position and prints a space
   at the current position */
void print_backspace() {
    set_cursor(--cursor_x, cursor_y);
    
    uint16_t offset = get_offset();
    uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

    videomem += offset;
    *videomem = (color << 8) | ' ';

    scroll();
}

/* prints char c at position x,y */
void print_at(char c, uint16_t x, uint16_t y) {
    if (x <= MAX_COLS && y < MAX_ROWS) {
        set_cursor(x, y);
        vga_print_char(c);
    }
}

/* prints string to the current cursor position */
void vga_print(char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vga_print_char(string[i]);
}

/* prints string to the current cursor position and then a newline */
void vga_println(char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vga_print_char(string[i]);
    vga_print_char('\n');
}

/* prints string to the position x,(alignment * 10) */
void vga_print_align(char *string, uint16_t alignment) {
    alignment = alignment % 8;  //mod 8 so that we don't align off screen
    set_cursor(alignment * 10, cursor_y);

    vga_print(string);
}

/* clears the screen */
void vga_clear_screen() {
    uint16_t space = (color << 8) | ' ';
    uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

    int i;
    for (i = 0; i < MAX_ROWS*MAX_COLS; i++)
        videomem[i] = space;
    
    set_cursor(0,0);
}

/* scrolls the screen */
void scroll() {
    if (cursor_y >= MAX_ROWS) {

        uint16_t space = (color << 8) | ' ';
        uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

        int i;
        for (i = 0; i < (MAX_ROWS - 1)*MAX_COLS; i++)
            videomem[i] = videomem[i + MAX_COLS];
        
        for (i = (MAX_ROWS - 1)*MAX_COLS; i < MAX_ROWS*MAX_COLS; i++)
            videomem[i] = space;
        
        set_cursor(0,24);
    }
}

/* copies the contents of the screen into buffer */
void vga_copy_screen(char *buffer) {
    memcpy((void *) buffer, (const void *) VIDEO_ADDRESS, MAX_COLS*MAX_ROWS);
}

void vga_set_fg_color(enum vga_color c) {
    color = (color & 0xff00) | c;
}

void vga_set_bg_color(enum vga_color c) {
    color = (color & 0x00ff) | c;
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    color = fg | bg << 4;
}

uint16_t vga_get_color() {
    return color;
}