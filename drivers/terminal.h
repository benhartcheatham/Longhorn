/* Defines the terminal interface and some extra terminal functionality */
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
    /* connected line discipline */
    struct line_discipline *ld;

    /* terminal state */
    struct terminal_state ts;

    /* connected display driver */
    struct display *dis;

    /** function to initialize a terminal
     * 
     * @param t: pointer to terminal to initialize
     * @param ld: pointer to line discipline to connect this terminal to
     * @param dd: pointer to display driver to connect this terminal to
     * 
     * @return implementation dependent
     */
    int (*term_init)(struct terminal *t, struct line_discipline *ld, struct display *dd);

    /** write a character to the terminal to be output to the display device
     * 
     * @param t: terminal to write to
     * @param c: character to write to terminal
     * 
     * @return implementation dependent
     */
    int (*term_write)(struct terminal *t, char c);

    /** write a null-terminated string to the terminal
     * not garunteed to be implemented
     * 
     * @param t: terminal to write to
     * @param s: null-terminated string to write to terminal
     * 
     * @return implementation dependent
     */
    int (*term_writes)(struct terminal *t, char *s);

    /** write a character to the terminal to be input to the connected line discipline
     * 
     * @param t: terminal to write to
     * @param c: character to write to terminal
     * 
     * @return implementation dependent
     */
    int (*term_in)(struct terminal *t, char c);

    /** write a null-terminated string to the terminal to be input to 
     * the connected line discipline
     * not garunteed to be implemented
     * 
     * @param t: terminal to write to
     * @param s: null-terminated string to write to terminal
     * 
     * @return implementation dependent
     */
    int (*term_ins)(struct terminal *t, char *s);

};

/* typedefs */
typedef struct terminal term_t;

/* functions */
int terminal_init(term_t *t, struct line_discipline *ld, struct display *dd);
term_t *get_default_terminal();

/** gets the terminal_state struct of a terminal
 * 
 * @param t: terminal to get state from
 * 
 * @return pointer to terminal state struct of terminal
 */
inline struct terminal_state *get_term_state(term_t *t) {
    return &t->ts;
}

#endif