#include <stdio.h>
#include <stdbool.h>
#include <kerrors.h>
#include <mem.h>
#include <string.h>
#include "terminal.h"
#include "../kernel/kalloc.h"

static term_t *dterm = NULL;


static int terminal_write(term_t *t, char c);
static int terminal_writes(term_t *t, char *s);
static int terminal_in(term_t *t, char c);
static int terminal_ins(term_t *t, char *s);

/* initialize the terminal */
int terminal_init(term_t *t, line_disc_t *ld, struct display *dd) {
    if (t == NULL || ld == NULL)
        return -TERM_INIT_FAIL;

    t->ts.capitalize = false;
    t->ts.alt_pressed = false;
    t->ts.ctrl_pressed = false;

    if (dd == NULL)
        t->dis = get_default_dis_driver();
    else
        t->dis = dd;
    
    t->ld = ld;

    t->term_init = NULL;
    t->term_write = terminal_write;
    t->term_writes = terminal_writes;
    t->term_in = terminal_in;
    t->term_ins = terminal_ins;

    if (dterm == NULL)
        dterm = t;
    
    return TERM_SUCC;
}

/* writes the given character (not keycode) to the screen */
static int terminal_write(term_t *t, char c) {
    t->dis->dis_putc(c);
    return TERM_SUCC;
}

/* writes the given strings (not keycodes) to the screen */
static int terminal_writes(term_t *t, char *s) {
    t->dis->dis_puts(s);
    return TERM_SUCC;
}

/* outputs the given keycode to the line discipline */
static int terminal_in(term_t *t, char c) {
    struct terminal_state *ts = get_term_state(t);
    if (ts == NULL)
        return -TERM_IN_FAIL;

    // don't let the line discipline see shift and such
    t->ld->line_in(t->ld, c);

    return TERM_SUCC;
}

/* outputs the given string of keycodes to the line discipline 
 * returns the number of chars written */
static int terminal_ins(term_t *t, char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++)
        if (t->term_in(t, s[i]) != TERM_SUCC)
            return i;
    
    return i;
}

term_t *get_default_terminal() {
    return dterm;
}

void set_default_terminal(term_t *t) {
    dterm = t;
}