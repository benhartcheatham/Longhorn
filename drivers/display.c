#include <stddef.h>
#include "display.h"
#include "vesa.h"
#include "../boot/multiboot.h"

dis_driver_t default_dd;

int display_init(void *aux) {
    multiboot_info_t *mbi = (multiboot_info_t *) aux;
    init_vesa(mbi);

    default_dd.init = NULL;
    default_dd.putc = vesa_print_char;
    default_dd.setcur = vesa_set_cursor;
    default_dd.getcurx = vesa_get_cursor_x;
    default_dd.getcury = vesa_get_cursor_y;
    default_dd.puti = NULL;
    default_dd.setcol = vesa_set_color;
    default_dd.getfcol = NULL;  // need to go back and implement these two
    default_dd.getbcol = NULL;
    default_dd.scur = vesa_show_cursor;
    default_dd.hcur = vesa_hide_cursor;
    default_dd.puts = vesa_print;
    default_dd.backspace = vesa_print_backspace;
    default_dd.clear = vesa_clear_screen;
    default_dd.putcp = NULL;    // need to go back and do this one as well

    return 0;
}

void set_display_driver(dis_driver_t *dd) {
    default_dd = *dd;
}

dis_driver_t *get_default_dd() {
    return &default_dd;
}