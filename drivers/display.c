#include <stddef.h>
#include "display.h"
#include "vesa.h"

static display_t default_dis;
static void display_putats(const char *s, uint32_t x, uint32_t y);

void init_display(void *aux) {
    default_dis.dis_init = NULL;
    default_dis.dis_setcur = vesa_set_cursor;
    default_dis.dis_getx = vesa_get_cursor_x;
    default_dis.dis_gety = vesa_get_cursor_y;
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

static void display_putats(const char *s, uint32_t x, uint32_t y) {
    default_dis.dis_setcur(x,y);
    default_dis.dis_puts(s);
}

display_t *get_default_dis_driver() {
    return &default_dis;
}