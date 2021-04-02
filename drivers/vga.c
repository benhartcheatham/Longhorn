/* Implements the VGA driver for writing text to the screen. This driver is currently deprecated in favor of the VESA driver. */

/* includes */
#include <stdint.h>
#include <mem.h>
#include "../kernel/port_io.h"
#include "vga.h"

/* defines */

/* globals */
static uint16_t cursor_x;
static uint16_t cursor_y;
static uint16_t color = DEFAULT_BACKGROUND;

/* prototypes */
void scroll();

/* functions */

/** --deprecated-- 
 * sets the cursor to x,y on the screen
 * 
 * @param x: x position to set the cursor to
 * @param y: y position to set the cursor to
 */
void vga_set_cursor(uint16_t x, uint16_t y) {

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

/** --deprecated--
 * makes the cursor visible
 */
void vga_show_cursor() {
    return;
}

/** --deprecated--
 * makes the cursor visible
 */
void vga_hide_cursor() {
    return;
}

/** --deprecated--
 * gets the offest of the cursor 
 * 
 * @return the offset of the cursor
 */
uint16_t get_offset() {
    uint32_t pos = 0;
    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((uint32_t) inb(0x3D5)) << 8;
    return pos;
}

/** --deprecated--
 * prints char c to the current cursor position 
 * 
 * @param c: ASCII character to print
 */
void vga_print_char(char c) {
    if (c == '\n') {
        cursor_y++;
        cursor_x = 0;
        vga_set_cursor(cursor_x, cursor_y);
        scroll();
        return;
    } else if (c == '\t') {
        int i;
        for (i = 0; i < 5 && cursor_x < MAX_COLS; i++) {
            uint16_t offset = get_offset();
            uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

            videomem += offset;
            *videomem = (color << 8) | ' ';

            cursor_x++;
            vga_set_cursor(cursor_x, cursor_y);
        }

        return;
    }

    uint16_t offset = get_offset();
    uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

    videomem += offset;
    *videomem = (color << 8) | c;

    cursor_x++;
    vga_set_cursor(cursor_x, cursor_y);
    scroll();
}

/** --deprecated--
 * removes the char before the current cursor position and prints a space
 * at the current position */
void vga_print_backspace() {
    vga_set_cursor(--cursor_x, cursor_y);
    
    uint16_t offset = get_offset();
    uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

    videomem += offset;
    *videomem = (color << 8) | ' ';

    scroll();
}

/** --deprecated-- 
 * prints char c at position x,y
 * 
 * @param c: ASCII character to write to the screen
 * @param x: x coordinate to write character to 
 * @param y: y coordinate to write character to 
 */
void print_at(char c, uint16_t x, uint16_t y) {
    if (x <= MAX_COLS && y < MAX_ROWS) {
        vga_set_cursor(x, y);
        vga_print_char(c);
    }
}

/** --deprecated--
 * prints string to the current cursor position
 * 
 * @param string: null-terminated string to write to screen
 */
void vga_print(char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vga_print_char(string[i]);
}

/** --deprecated--
 * prints string to the current cursor position and then a newline 
 * 
 * @param string: null-terminated string to write to screen
 */
void vga_println(char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vga_print_char(string[i]);
    vga_print_char('\n');
}

/** --deprecated--
 * prints string to the position x,(alignment * 10)
 * 
 * @param string: null-terminated string to write to screen
 * @param alignment: alingment of string
 */
void vga_print_align(char *string, uint16_t alignment) {
    alignment = alignment % 8;  //mod 8 so that we don't align off screen
    vga_set_cursor(alignment * 10, cursor_y);

    vga_print(string);
}

/** --deprecated--
 * clears the screen */
void vga_clear_screen() {
    uint16_t space = (color << 8) | ' ';
    uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

    int i;
    for (i = 0; i < MAX_ROWS*MAX_COLS; i++)
        videomem[i] = space;
    
    vga_set_cursor(0,0);
}

/** --deprecated--
 * scrolls the screen */
void scroll() {
    if (cursor_y >= MAX_ROWS) {

        uint16_t space = (color << 8) | ' ';
        uint16_t *videomem = (uint16_t *) VIDEO_ADDRESS;

        int i;
        for (i = 0; i < (MAX_ROWS - 1) * MAX_COLS; i++)
            videomem[i] = videomem[i + MAX_COLS];
        
        for (i = (MAX_ROWS - 1) * MAX_COLS; i < MAX_ROWS*MAX_COLS; i++)
            videomem[i] = space;
        
        vga_set_cursor(0,24);
    }
}

/** --deprecated--
 * copies the contents of the screen into buffer 
 * 
 * @param buffer: buffer to copy framebuffer into
 */
void vga_copy_screen(char *buffer) {
    memcpy((void *) buffer, (const void *) VIDEO_ADDRESS, MAX_COLS*MAX_ROWS);
}

/** --deprecated--
 * sets the foreground color of the text
 * 
 * @param c: color to set text to
 */
void vga_set_fg_color(enum vga_color c) {
    color = (color & 0xff00) | c;
}

/** --deprecated--
 * sets the background color of the text
 * 
 * @param c: color to set background of text to
 */
void vga_set_bg_color(enum vga_color c) {
    color = (color & 0x00ff) | c;
}

/** --deprecated--
 * sets the color of the text
 * 
 * @param fg: color to set text to
 * @param bg: color to set background of text to
 */
void vga_set_color(enum vga_color fg, enum vga_color bg) {
    color = fg | bg << 4;
}

/** --deprecated--
 * gets the color of text
 * 
 * @return color of text
 */
uint16_t vga_get_color() {
    return color;
}