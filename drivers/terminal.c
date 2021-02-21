#include <stdio.h>
#include <stdbool.h>
#include <kerrors.h>
#include "terminal.h"
#include "../kernel/kalloc.h"

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

static struct terminal dterm;

struct terminal_state {
    bool capitalize;
    bool alt_pressed;
    bool ctrl_pressed;
};

static int terminal_init(term_t *t, line_disc_t *ld, struct display *dd);
static int terminal_write(term_t *t, char c);
static int terminal_writes(term_t *t, char *s);
static int terminal_writebuf(term_t *t);
static int terminal_in(term_t *t, char c);
static int terminal_ins(term_t *t, char *s);
static int terminal_outc(term_t *t);
static int terminal_outs(term_t *t);
static bool is_whitespace(uint8_t keycode);

static int terminal_init(term_t *t, line_disc_t *ld, struct display *dd) {
    if (t == NULL || ld == NULL)
        return TERM_INIT_FAIL;
    
    struct terminal_state *ts = kmalloc(sizeof(struct terminal_state));
    t->in = kcalloc(TERMINAL_BUFF_SIZE, sizeof(TERMINAL_BUFF_TYPE));
    t->out = kcalloc(TERMINAL_BUFF_SIZE, sizeof(TERMINAL_BUFF_TYPE));

    if (ts == NULL || t->in == NULL || t->out == NULL)
        return TERM_INIT_FAIL;

    ts->capitalize = false;
    ts->alt_pressed = false;
    ts->ctrl_pressed = false;

    if (dd == NULL)
        t->dis = get_default_dis_driver();
    else
        t->dis = dd;
    
    t->ld = ld;
    return TERM_SUCC;
}



static bool is_whitespace(uint8_t keycode) {
    if (keycode == KC_LALT || keycode == KC_LSHIFT || 
            keycode == KC_RELEASED || keycode == KC_CAPSLOCK || keycode == KC_ENTER)
        return true;
    return false;
}