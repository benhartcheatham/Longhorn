#include <stdio.h>
#include <stdbool.h>
#include <kerrors.h>
#include <mem.h>
#include <string.h>
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
    uint32_t index;

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
static int terminal_outs(term_t *t);
static int terminal_flush(term_t *t, uint32_t from);
static inline struct terminal_state *get_term_state(term_t *t);
static char eval_kc(term_t *t, char keycode);

static int terminal_init(term_t *t, line_disc_t *ld, struct display *dd) {
    if (t == NULL || ld == NULL)
        return TERM_INIT_FAIL;
    
    struct terminal_state *ts = kmalloc(sizeof(struct terminal_state));
    t->in = kcalloc(TERMINAL_BUFF_SIZE + 1, sizeof(TERMINAL_BUFF_TYPE));

    if (ts == NULL || t->in == NULL)
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

static int terminal_write(term_t *t, char c) {
    t->dis->dis_putc(c);
    return TERM_SUCC;
}

static int terminal_writes(term_t *t, char *s) {
    t->dis->dis_puts(s);
    return TERM_SUCC;
}

static int terminal_writebuf(term_t *t) {
    t->dis->dis_puts(t->in);
    return TERM_SUCC;
}

static int terminal_in(term_t *t, char c) {
    struct terminal_state *ts = get_term_state(t);
    if (ts == NULL)
        return TERM_IN_FAIL;
    
    if (ts->index > TERMINAL_BUFF_SIZE || ts->index < 0)
        return TERM_IN_FAIL;
    
    if (eval_kc(t, c) == 0)
        return TERM_SUCC;
    
    if (c == KC_BACKSPACE) {
        t->in[ts->index--] = 0;
        return TERM_SUCC;
    }

    if (c == KC_ENTER) {
        t->term_outs(t);
        return TERM_SUCC;
    }

    t->in[ts->index++] = c;
    t->term_write(t, c);
    return TERM_SUCC;
}

static int terminal_ins(term_t *t, char *s) {
    struct terminal_state *ts = get_term_state(t);
    uint32_t begin_index = 0;

    if (ts != NULL)
        begin_index = ts->index;
    
    for (int i = 0; s[i] != '\0'; i++)
        if (t->term_in(t, s[i]) != TERM_SUCC)
            terminal_flush(t, begin_index);
    
    return TERM_SUCC;
}

static int terminal_outs(term_t *t) {
    t->ld->line_ins(t->ld, t->in);

    terminal_flush(t, 0);

    return TERM_SUCC;
}


static int terminal_flush(term_t *t, uint32_t from) {
    for (int i = from; i < TERMINAL_BUFF_SIZE; i++)
        t->in[i] = 0;

    struct terminal_state *ts = get_term_state(t);
    if (ts)
        ts->index = from;
    
    return TERM_SUCC;
}

static inline struct terminal_state *get_term_state(term_t *t) {
    return (struct terminal_state *) t->term_state;
}

static char eval_kc(term_t *t, char keycode) {
    struct terminal_state *ts = get_term_state(t);

    if (ts == NULL)
        return 0;
    
    switch (keycode) {
        case KC_LALT:
            ts->alt_pressed = !ts->alt_pressed;
            return 0;
        case KC_LSHIFT:
        case KC_RSHIFT:
        case KC_CAPSLOCK:
            ts->capitalize = !ts->capitalize;
            return 0;
        case KC_LCTRL:
            ts->ctrl_pressed = !ts->ctrl_pressed;
            return 0;
        case KC_BACKSPACE:
            if (ts->index > 0) {
                t->dis->dis_backspace();
                return KC_BACKSPACE;
            }
            return 0;
        case KC_ESCAPE:
        case KC_NUMLOCK:
        case KC_RELEASED:
            return 0;
        default:
            return keycode;
    }
}