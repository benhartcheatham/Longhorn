/* Implements the VESA driver for drawing text and single pixels to the screen. 
 * Writes directly to the framebuffer and does no buffering itself */

/* includes */
#include <stdint.h>
#include "../boot/multiboot.h"
#include "../kernel/port_io.h"
#include "vesa.h"
#include "vga_font.h"

/* globals */
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
static uint32_t cursor_on;
static uint32_t current_x;
static uint32_t current_y;

static uint32_t bg_color;
static uint32_t fg_color;

/* prototypes */
static void vesa_set_cursor_vis(uint8_t state);
static void scroll();

/* functions */

/** Initialize the vesa driver 
 * 
 * @param mbi: pointer to the mbi given by GRUB2
 */
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

    bg_color = BLACK;
    fg_color = WHITE;
}

/** sets the position of the cursor
 * 
 * @param x: x coordinate to set the cursor to
 * @param y: y coordinate to set the cursor to
 */
void vesa_set_cursor(uint32_t x, uint32_t y) {
    if (y <= num_rows && x <= num_cols) {
        current_x = FONT_WIDTH * x;
        cursor_x = x;

        current_y = FONT_HEIGHT * y;
        cursor_y = y;
    }
}

/** gets the x coordinate of the cursor
 * 
 * @return x coordinate of cursor
 */
uint32_t vesa_get_cursor_x() {
    return cursor_x;
}

/** gets the y coordinate of the cursor
 * 
 * @return y coordinate of cursor
 */
uint32_t vesa_get_cursor_y() {
    return cursor_y;
}

/** gets whether the cursor is currently visible
 * 
 * @return 1 if cursor is visble, 0 otherwise
 */
uint32_t vesa_get_cursor_vis() {
    return cursor_on;
}

/** sets the visibility of the cursor
 * 
 * @param state: state to set the cursor to: 1 for on, 0 for off
 */
static void vesa_set_cursor_vis(uint8_t state) {
    if (state == 1)
        vesa_show_cursor();
    else
        vesa_hide_cursor();
}

/** gets the number of cursor columns
 * 
 * @return number of cursor columns
 */
uint32_t vesa_get_num_cols() {
    return num_cols;
}

/** gets the number of cursor rows
 * 
 * @return number of cursor rows
 */
uint32_t vesa_get_num_rows() {
    return num_rows;
}

/** turns on the cursor */
void vesa_show_cursor() {
    uint32_t *pixel_pos = framebuffer_addr + (current_y * width) + current_x;

    int i;
    for (i = 0; i < FONT_HEIGHT; i++) {
        char c_font = (char) vga_font[(219 * FONT_HEIGHT) + i];

        int j;
        for (j = 0; j < FONT_WIDTH; j++)
            if (c_font & (1 << j))
                pixel_pos[FONT_WIDTH - j] = fg_color;
        
        pixel_pos += width;
    }

    cursor_on = 1;
}

/** hides the cursor */
void vesa_hide_cursor() {
    uint32_t *pixel_pos = framebuffer_addr + (current_y * width) + current_x;

    int i;
    for (i = 0; i < FONT_HEIGHT; i++) {

        int j;
        for (j = 0; j < FONT_WIDTH; j++)
            pixel_pos[FONT_WIDTH - j] = bg_color;
        
        pixel_pos += width;
    }

    cursor_on = 0;
}

/** prints an ASCII character to the current cursor position
 * 
 * @param c: character to print
 */
void vesa_print_char(char c) {
    if (c == '\n') {
        vesa_hide_cursor();
        vesa_set_cursor(0, cursor_y + 1);
        scroll();
        return;
    } else if (c == '\t') {
        vesa_hide_cursor();

        if (cursor_x <= num_cols - 4)
            vesa_set_cursor(cursor_x + 4, cursor_y);
        else
            vesa_set_cursor(cursor_x + (num_cols - 4), cursor_y);

        return;
    }

    uint32_t *pixel_pos = framebuffer_addr + (current_y * width) + current_x;

    int i;
    for (i = 0; i < FONT_HEIGHT; i++) {
        char c_font = (char) vga_font[(c * FONT_HEIGHT) + i];

        int j;
        for (j = 0; j < FONT_WIDTH; j++) {
            if (c_font & (1 << j))
                pixel_pos[FONT_WIDTH - j] = fg_color;
            else
                pixel_pos[FONT_WIDTH - j] = bg_color;
        }
        
        pixel_pos += width;
    }

    current_x += FONT_WIDTH;
    cursor_x++;
    scroll();
}

/** prints a backspace to the current cursor position */
void vesa_print_backspace() {
    uint8_t old_vis = vesa_get_cursor_vis();
    vesa_hide_cursor();
    vesa_set_cursor(--cursor_x, cursor_y);

    uint32_t *pixel_pos = framebuffer_addr + (current_y * width) + current_x;

    int i;
    for (i = 0; i < FONT_HEIGHT; i++) {

        int j;
        for (j = 0; j < FONT_WIDTH; j++)
            pixel_pos[FONT_WIDTH - j] = bg_color;
        
        pixel_pos += width;
    }

    vesa_set_cursor_vis(old_vis);

    scroll();
}

/** prints a null-terminated string to the current cursor position
 * 
 * @param s: null-terminated string to print
 */
void vesa_print(const char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vesa_print_char(string[i]);
}

/** prints a null-terminated string to the current cursor position
 * with an ensuing newline
 * 
 * @param s: null-terminated string to print
 */
void vesa_println(const char *string) {
    int i;
    for (i = 0; string[i] != 0; i++)
        vesa_print_char(string[i]);
    vesa_print_char('\n');
}

/** draws a pixel on the screen
 * 
 * @param x: x coordinate to draw at
 * @param y: y coordinate to draw at
 * @param col: color to draw as a 32 bit RGB value
 */
void vesa_draw(uint32_t x, uint32_t y, uint32_t col) {
    uint32_t *pixel_pos = framebuffer_addr + (y * width) + x;
    *pixel_pos = col;
}

/** clears the screen */
void vesa_clear_screen() {
    uint32_t x;
    uint32_t y;
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            framebuffer_addr[(y * height) + x] = bg_color;
    
    vesa_set_cursor(0, 0);
}

/** scrolls the screen */
static void scroll() {
    if (cursor_x >= num_cols) {
        current_y += FONT_HEIGHT;
        cursor_y++;
        current_x = 0;
        cursor_x = 0;
    }
    
    if (cursor_y >= num_rows) {
        uint32_t x;
        uint32_t y;
        for (y = 0; y < height - 1; y++)
            for (x = 0; x < width; x++)
                framebuffer_addr[(y * width) + x] = framebuffer_addr[((y  + FONT_HEIGHT) * width) + x];
    
        vesa_set_cursor(0, num_rows - 1);
    }
}

/** sets the color to write text to the screen in
 * 
 * @param fg: foreground color to write in as a 32 bit RGB value
 * @param bg: background color to write in as a 32 bit RGB value
 */
void vesa_set_color(uint32_t fg, uint32_t bg) {
    fg_color = fg;
    bg_color = bg;
}

/** sets the foreground color to write text to the screen in
 * 
 * @param fg: foreground color to write in as a 32 bit RGB value
 */
void vesa_set_fg_color(uint32_t fg) {
    fg_color = fg;
}

/** sets the background color to write text to the screen in
 * 
 * @param fg: background color to write in as a 32 bit RGB value
 */
void vesa_set_bg_color(uint32_t bg) {
    bg_color = bg;
}

/** sets the color to white foreground on a black background for text */
void vesa_set_default_color() {
    fg_color = WHITE;
    bg_color = BLACK;
}
