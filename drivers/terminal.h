#ifndef _TERMINAL_H
#define _TERMINAL_H

/* includes */
#include <stdint.h>
#include <stdbool.h>
#include "keyboard.h"
#include "display.h"
#include "line.h"

/* defines */
#define TERMINAL_BUFF_SIZE 1023
#define TERMINAL_BUFF_TYPE char
#define TAB_WIDTH 4

#define KC_MAX 57
#define KC_TAB SC_TAB
#define KC_LALT SC_LALT
#define KC_LSHIFT SC_LSHIFT
#define KC_LCTRL SC_LCTRL
#define KC_CAPSLOCK SC_CAPSLOCK
#define KC_RSHIFT SC_RSHIFT
#define KC_ENTER SC_ENTER
#define KC_BACKSPACE SC_BACKSPACE
#define KC_ESCAPE SC_ESCAPE
#define KC_NUMLOCK SC_NUMLOCK
#define KC_RELEASED SC_RELEASED

#define ASCII_BACKSPACE 8
#define ASCII_HTAB 9
#define ASCII_NEWLINE 10
#define ASCII_CRETURN 15

/* structs */
struct terminal_state {
    bool capitalize;
    bool alt_pressed;
    bool ctrl_pressed;
};

struct terminal {
// connected line discipline
struct line_discipline *ld;

// state
struct terminal_state ts;

// display driver
struct display *dis;

// input driver - not implemented

// initialize the terminal
int (*term_init)(struct terminal *t, struct line_discipline *ld, struct display *dd);

// write to the screen
int (*term_write)(struct terminal *t, char c);
int (*term_writes)(struct terminal *t, char *s);    // doesn't have to be implemented

// input into terminal from keyboard driver
int (*term_in)(struct terminal *t, char c);
int (*term_ins)(struct terminal *t, char *s);   // doesn't have to be implemented

};

/* typedefs */
typedef struct terminal term_t;

/* functions */
int terminal_init(term_t *t, struct line_discipline *ld, struct display *dd);
term_t *get_default_terminal();
void set_default_terminal(term_t *t);

inline struct terminal_state *get_term_state(term_t *t) {
    return &t->ts;
}

#endif