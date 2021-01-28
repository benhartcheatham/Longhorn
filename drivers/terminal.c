/* Terminal is only meant for input from the keyboard currently,
 * this should be expanded to be any hardware device later on.
 * There can also only be one terminal on the machine at once,
 * this should probably be changed. */

#include "terminal.h"
#include "vesa.h"
#include "vga.h"
#include <stdio.h>

static const char kc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
                          '7', '8', '9', '0', '-', '=', '?', '?', 
                          'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
                          'o', 'p', '[', ']', '\n', '?', 'a', 's', 
                          'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
                          '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
                          'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

static const char kc_ascii_cap[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '[', ']', '\n', '?', 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

static uint32_t next_id = 0;
static struct terminal *active_t = NULL;
static int8_t capitalize = 0; 

static int is_whitespace(uint8_t keycode);

void terminal_init(struct terminal *t) {
    t->out = PROC_CUR();
    t->mode = FILTERED;
    t->drmode = VESA;
    t->dmode = D_FILTERED;

    t->term_id = next_id;
    next_id++;

    if (active_t == NULL)
        active_t = t;
}

int terminal_out(struct terminal *t, struct process *out) {
    if (out == NULL || t == NULL)
        return -1;
    
    t->out = out;
    return 0;
}

int terminal_mode(struct terminal *t, tmode_t mode) {
    if (mode > sizeof(enum TERMINAL_MODE))
        return -1;
    
    t->mode = mode;
    return 0;
}

int terminal_dmode(struct terminal *t, tdmode_t mode) {
    if (t == NULL || mode > sizeof(enum TERMINAL_DIS_MODE))
        return -1;

    if (t->drmode != VESA && t->drmode != VGA)
        return -2;
    
    t->dmode = mode;
    return 0;
}

int terminal_drmode(struct terminal *t, tdrmode_t mode) {
    if (mode > sizeof(enum TERMINAL_DRIVER_MODE))
        return -1;

    t->drmode = mode;
    return 0;
}

int terminal_active(struct terminal *t) {
    if (t == NULL)
        return -1;
    
    active_t = t;
    return 0;
}

/* puts the char into the stdin of out and sends the char to the 
   configured driver */
int terminal_put(uint8_t c) {
    if (active_t == NULL)
        return -1;

    if (active_t->mode == FILTERED) {
        if (c == ENTER) {
            put_std(&active_t->out->stdin, '\n');

        } else if (c == BACKSPACE) {
            put_std(&active_t->out->stdin, '\b');

        } else if (c == SHIFT_PRESSED || c == SHIFT_RELEASED || c == CAPS_LOCK_PRESSED) {
            capitalize = !capitalize;

        } else if (c <= KC_MAX && c > 0) {
            if (capitalize == 0)
                put_std(&active_t->out->stdin, kc_ascii[c]);
            else
                put_std(&active_t->out->stdin, kc_ascii_cap[c]);
        }
    } else if (active_t->mode == RAW) {
        put_std(&active_t->out->stdin, (char) c);
    }

    if (active_t->dmode == D_RAW) {
        if (active_t->drmode == VESA)
            vesa_print_char((char) c);
        else if (active_t->drmode == VGA)
            vga_print_char((char) c);
    
    } else if (active_t->dmode == D_FILTERED) {
        if (c == BACKSPACE) {
            if (active_t->drmode == VESA)
                vesa_print_backspace();
            else if (active_t->drmode == VGA)
                vga_print_backspace();
        } else if (c == SHIFT_PRESSED || c == SHIFT_RELEASED || c == CAPS_LOCK_PRESSED) {
            capitalize = !capitalize;

        }
        
    } 
    
    if (active_t->dmode == D_FILTERED || active_t->dmode == D_CHAR_ONLY) { 
        if (c <= KC_MAX && c > 0 && !is_whitespace(c)) {
            if (active_t->drmode == VESA) {
                if (capitalize == 1)
                    vesa_print_char(kc_ascii_cap[c]);
                else if (capitalize == 0)
                    vesa_print_char(kc_ascii[c]);
            } else if (active_t->drmode == VGA) {
                if (capitalize == 1)
                    vga_print_char(kc_ascii_cap[c]);
                else if (capitalize == 0)
                    vga_print_char(kc_ascii[c]);
            }

        }
    } else {
        return -1;
    }
    
    return 0;
}

int terminal_scur() {
    if (active_t->drmode != VESA)
        return -1;
    
    vesa_show_cursor();
    return 0;
}

int terminal_hcur() {
    if (active_t->drmode != VESA)
        return -1;
    
    vesa_hide_cursor();
    return 0;
}

int terminal_p(char *str) {
    if (active_t->drmode == VESA)
        vesa_print(str);
    else if (active_t->drmode == VGA)
        vga_print(str);
    else 
        return -1;
    
    return 0;
}

int terminal_pln(char *str) {
    if (active_t->drmode == VESA)
        vesa_println(str);
    else if (active_t->drmode == VGA)
        vga_println(str);
    else 
        return -1;
    
    return 0;
}

int terminal_pback() {
    if (active_t->drmode == VESA)
        vesa_print_backspace();
    else if (active_t->drmode == VGA)
        vga_print_backspace();
    else
        return -1;
    
    return 0;
}

int terminal_fgc(uint32_t col) {
    if (active_t->drmode == VESA)
        vesa_set_fg_color(col);
    else if (active_t->drmode == VGA)
        vga_set_fg_color((enum vga_color) col);
    else
        return -1;

    return 0;
}

int terminal_bgc(uint32_t col) {
    if (active_t->drmode == VESA)
        vesa_set_bg_color(col);
    else if (active_t->drmode == VGA)
        vga_set_bg_color((enum vga_color) col);
    else
        return -1;

    return 0;
}

static int is_whitespace(uint8_t keycode) {
    if (keycode == BACKSPACE || keycode == SHIFT_PRESSED || 
            keycode == SHIFT_RELEASED || keycode == CAPS_LOCK_PRESSED || keycode == ENTER)
        return 1;
    return 0;
}