/* THIS LINE DISCIPLINE IS MEANT FOR A COOKED MODE TERMINAL,
   IT DOES NO ACTUAL LINE FILTERING */

#include <kerrors.h>
#include <mem.h>
#include <string.h>
#include "line.h"
#include "../kernel/kalloc.h"

static struct line_discipline dline;

const char kc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
                          '7', '8', '9', '0', '-', '=', '?', '?', 
                          'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
                          'o', 'p', '[', ']', '\n', '?', 'a', 's', 
                          'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
                          '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
                          'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

const char kc_ascii_cap[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '[', ']', '\n', '?', 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

static int line_in(line_disc_t *ld, char c);
static size_t line_ins(line_disc_t *ld, char *s);
static size_t line_out(line_disc_t *ld);
static size_t line_send(line_disc_t *ld);
static int line_sendv(line_disc_t *ld);
static int line_recv(line_disc_t *ld, char c);
static size_t line_recs(line_disc_t *ld, char *s);
static int line_flush(line_disc_t *ld);

static char eval_kc(term_t *t, char keycode);
static char kc_getchar(term_t *t, unsigned char c);

/* init the line discipline */
int line_init(line_disc_t *ld, std_stream *out, ld_modes_t m) {
    if (ld == NULL)
        return -LINE_INIT_FAIL;
    
    ld->line_buffer = kcalloc(LINE_BUFFER_SIZE, sizeof(LINE_BUFFER_SIZE));
    ld->buffer_i = 0;
    if (ld->line_buffer == NULL)
        return -LINE_INIT_FAIL;
    
    ld->term = (term_t *) kmalloc(sizeof(term_t));   // setup terminal
    terminal_init(ld->term, ld, NULL);  // use default display driver

    if (out)
        ld->out = out;
    
    ld->mode = m;

    ld->line_init = NULL;
    ld->line_flush = line_flush;
    ld->line_in = line_in;
    ld->line_ins = line_ins;
    ld->line_out = line_out;
    ld->line_send = line_send;
    ld->line_sendv = line_sendv;
    ld->line_recv = line_recv;
    ld->line_recs = line_recs;
    
    return LINE_SUCC;
}

/* flushes the line buffer */
static int line_flush(line_disc_t *ld) {
    if (ld->line_buffer == NULL)
        return -LINE_INIT_FAIL;
    
    for (size_t i = 0; i < ld->buffer_i && i < LINE_BUFFER_SIZE; i++)
        ld->line_buffer[i] = 0;

    ld->buffer_i = 0;
    return LINE_SUCC;
}

/* writes a KC to the line buffer to be output 
 * also sends the corresponding char to the connected process
 * returns the amount of chars written */
static int line_in(line_disc_t *ld, char c) {
    // update terminal state
    char kc_ret = eval_kc(ld->term, c);

    if (ld->buffer_i == LINE_BUFFER_SIZE - 1)
        return -LINE_IN_FAIL;
    
    if (ld->mode == RAW) {
        // put char into buffer and send to process
        c = kc_getchar(ld->term, c);
        ld->line_buffer[ld->buffer_i++] = c;
        line_sendv(ld);

    } else if (ld->mode == COOKED) {
        if (!kc_ret)
            return LINE_SUCC;
        
        if (c == KC_BACKSPACE) {
            if (ld->buffer_i > 0) {
                ld->line_buffer[ld->buffer_i--] = '\0';
                ld->term->dis->dis_backspace(); // should have a function in terminal for this
                return LINE_SUCC;
            }
        }

        c = kc_getchar(ld->term, c);
        ld->line_buffer[ld->buffer_i++] = c;
        line_sendv(ld);
    }

    ld->term->term_write(ld->term, ld->line_buffer[ld->buffer_i]);
    
    return LINE_SUCC;
}

/* writes a string of keycodes to the line buffer to be output 
 * also sends the corresponding string to the connected process
 * returns the amount of chars written */
static size_t line_ins(line_disc_t *ld, char *s) {
    size_t i;
    for (i = 0; i < strlen(s) && i < LINE_BUFFER_SIZE; i++)
        if (line_in(ld, s[i]) == -LINE_IN_FAIL)
            return i;
    
    return i;
}

/* writes the line buffer to the terminal
 * returns the amount of chars written
 * doesn't flush the line buffer */
static size_t line_out(line_disc_t *ld) {
    size_t i;
    for (i = 0; i < LINE_BUFFER_SIZE && i < ld->buffer_i; i++) {
        int ret = LINE_SUCC;

        if (ld->line_buffer[i] == '\b')
            ld->term->dis->dis_backspace();
        else
            ret = ld->term->term_write(ld->term, ld->line_buffer[i]);
        
        if (ret != LINE_SUCC)
            return i;
    }

    return i;
}

// /* writes a char directly to the terminal from a process 
//  * returns LINE_SUCC on success, failure otherwise */
// static int line_proc_in(line_disc_t *ld, char c) {
//     if (c == '\b') {
//         ld->term->dis->dis_backspace();
//         return LINE_SUCC;
//     }

//     return ld->term->term_write(ld->term, c);
// }

// /* write a string directly to the terminal from a process 
//  * returns amount of chars written */
// static int line_proc_ins(line_disc_t *ld, char *s) {
//     int i;
//     for (i = 0; i < ld->buffer_i && i < LINE_BUFFER_SIZE)
//         if (ld->line_proc_in(ld, s[i]) != LINE_SUCC)
//             return i;
    
//     return i;
// }

/* send the entire line_buffer to the connected process
 * doesn't output the buffer to the terminal 
 * returns number of chars written */
static size_t line_send(line_disc_t *ld) {
    size_t i;
    for (i = 0; i < ld->buffer_i && i < LINE_BUFFER_SIZE; i++) {
        put_std(ld->out, ld->line_buffer[i]);
        ld->line_buffer[i] = 0;
    }
    
    ld->buffer_i = 0;
    return i;
}

/* send a char from the line_buffer to the connected process
 * doesn't output the character to the terminal */
static int line_sendv(line_disc_t *ld) {
    if (ld->buffer_i > 0) {
        put_std(ld->out, ld->line_buffer[ld->buffer_i]);
        ld->line_buffer[ld->buffer_i--] = 0;
        return LINE_SUCC;
    }

    return -LINE_OUT_FAIL;
}

/* input a char into line buffer from a process
 * char shouldn't be a KC, but an actual character
 * returns LINE_SUCC on success, otherwise failure */
static int line_recv(line_disc_t *ld, char c) {
    if (ld->buffer_i == LINE_BUFFER_SIZE - 1)
        return -LINE_IN_FAIL;
        
    if (c == '\b') {
        if (ld->buffer_i > 0) {
            ld->line_buffer[ld->buffer_i--] = '\0';
        }
    }

    ld->line_buffer[ld->buffer_i++] = c;
    return LINE_SUCC;
}

/* input a string into line buffer from a process
 * char shouldn't be a KC, but an actual character
 * only inputs the string until the end of the buffer is reached
 * returns the number of chars written */
static size_t line_recs(line_disc_t *ld, char *s) {
    // calculate amount of buffer left
    size_t s_size = strlen(s) < LINE_BUFFER_SIZE - ld->buffer_i  ? strlen(s) : LINE_BUFFER_SIZE - ld->buffer_i + 1;
    // copy string
    memcpy(ld->line_buffer + ld->buffer_i, s, s_size);
    // update buffer
    ld->buffer_i += s_size;
    ld->line_buffer[ld->buffer_i] = 0;
    return s_size;
}

line_disc_t *get_default_line_disc() {
    return &dline;
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
        case KC_ESCAPE:
        case KC_NUMLOCK:
        case KC_RELEASED:
            return 0;
        default:
            return keycode;
    }
}

static char kc_getchar(term_t *t, unsigned char c) {
    struct terminal_state *ts = get_term_state(t);
    if (ts == NULL)
        return 0;
    
    if (ts->capitalize == true) 
        return kc_ascii_cap[c];
    else
        return kc_ascii[c];
}