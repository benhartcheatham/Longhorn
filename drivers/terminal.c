/* Default implementation of the terminal interface with some extra functionality */

/* includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <mem.h>
#include <kerrors.h>
#include "../kernel/kalloc.h"
#include "terminal.h"

/* defines */

/* globals */
static term_t *dterm = NULL;

/* prototypes */
static int terminal_write(term_t *t, char c);
static int terminal_writes(term_t *t, char *s);
static int terminal_in(term_t *t, char c);
static int terminal_ins(term_t *t, char *s);

/* functions */

/** initializes a terminal
 * 
 * @param t: pointer to terminal to initialize, must be non-NULL
 * @param ld: pointer to ld to connect terminal to, must be non-NULL
 * @param dd: display driver to connect terminal to, must be non-NULL
 * 
 * @return -TERM_INIT_FAIL on failure, TERM_SUCC otherwise
 */
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

/** writes the given ASCII character to the screen 
 * 
 * @param t: terminal to write to
 * @param c: character to write to screen
 * 
 * @return TERM_SUCC
 */
static int terminal_write(term_t *t, char c) {
    t->dis->dis_putc(c);
    return TERM_SUCC;
}

/** writes the given null-terminated ASCII string to the screen 
 * 
 * @param t: terminal to write to
 * @param s: null-terminated string to write to screen
 * 
 * @return TERM_SUCC
 */
static int terminal_writes(term_t *t, char *s) {
    t->dis->dis_puts(s);
    return TERM_SUCC;
}

/** outputs the given keycode to the line discipline 
 * 
 * @param t: terminal to write from
 * @param c: keycode to write to t's connected line discipline
 * 
 * @return TERM_SUCC
 */
static int terminal_in(term_t *t, char c) {
    t->ld->line_in(t->ld, c);
    return TERM_SUCC;
}

/** outputs the given null-terminated string of keycodes to the line discipline 
 * 
 * @param t: terminal to write from
 * @param s: null-terminated string of keycodes to write to t's connected line discipline
 * 
 * @return number of characters written
 */
static int terminal_ins(term_t *t, char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++)
        if (t->term_in(t, s[i]) != TERM_SUCC)
            return i;
    
    return i;
}

/** gets the terminal connected to default line discipline
 * 
 * @return a pointer to the default terminal
 */
term_t *get_default_terminal() {
    return dterm;
}