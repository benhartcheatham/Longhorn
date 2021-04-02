/* Implements the default VESA display driver for the system. */

/* includes */
#include <stddef.h>
#include "display.h"
#include "vesa.h"

/* prototypes */
static display_t default_dis;
static void display_putats(const char *s, uint32_t x, uint32_t y);

/* functions */

/** Intializes the default VESA display driver 
 * 
 * @param aux: a pointer to the mbi struct given by GRUB2
 */
void display_init(void *aux) {
    default_dis.dis_init = NULL;
    default_dis.dis_setcur = vesa_set_cursor;
    default_dis.dis_getcur_vis = vesa_get_cursor_vis;
    default_dis.dis_getx = vesa_get_cursor_x;
    default_dis.dis_gety = vesa_get_cursor_y;
    default_dis.dis_getn_cols = vesa_get_num_cols;
    default_dis.dis_getn_rows = vesa_get_num_rows;
    default_dis.dis_scur = vesa_show_cursor;
    default_dis.dis_hcur = vesa_hide_cursor;
    default_dis.dis_putc = vesa_print_char;
    default_dis.dis_puts = vesa_print;
    default_dis.dis_putats = display_putats;
    default_dis.dis_backspace = vesa_print_backspace;
    default_dis.dis_draw = NULL;    // not implemented
    default_dis.dis_clear = vesa_clear_screen;
    default_dis.dis_setcol = vesa_set_color;
    init_vesa(aux);
}

/** utility function for putting a string at location (x,y) on the screen
 * 
 * @param s: null-terminated string to be printed
 * @param x: x coordinate (in cursor coordinates) to write the string
 * @param y: y coordinate (in cursor coordinates) to write the string
 */
static void display_putats(const char *s, uint32_t x, uint32_t y) {
    default_dis.dis_setcur(x,y);
    default_dis.dis_puts(s);
}

/** returns the default display driver initialized by the system 
 * 
 * @return a pointer to the default display driver
 */
display_t *get_default_dis_driver() {
    return &default_dis;
}