/* Terminal is only meant for input from the keyboard currently,
 * this should be expanded to be any hardware device later on.
 * There can also only be one terminal on the machine at once,
 * this should probably be changed. */

#include "term.h"
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

static int8_t capitalize = 0;

static term_t default_term;

static int terminal_flush(term_t *t);
static int is_whitespace(char c);

term_t terminal_init(key_modes_t mode, std_stream *in, key_driver_t *kd, dis_driver_t *dd, void *aux) {
    term_t temp;
    temp.terminal_init = terminal_init;
    temp.terminal_mode = terminal_mode;

    temp.kd = kd;
    temp.dd = dd;
    temp.terminal_mode(&temp, mode);
    temp.buff_i = 0;
    
    // a buffer must be supplied for RAW mode, otherwise there is
    // undefined behavior
    if (in != NULL && mode == RAW)
        temp.reg_buff = in;
    else
        temp.reg_buff = NULL;
    
    temp.reg_key = 0;

    temp.kd->keyboard_mode(mode);
    return temp;
}

int terminal_mode(term_t *t, key_modes_t mode) {
    if (t) {
        t->mode = mode;
        t->kd->keyboard_mode(mode);
    } else
        return -1;
    
    return 0;
}

int terminal_register(term_t *t, std_stream *in, char c) {
    // this feature is only available in COOKED mode
    if (t->mode != COOKED)
        return -1;
    
    t->reg_key = c;
    t->reg_buff = in;
    return 0;
}

char terminal_getc() {

}

int terminal_writec(term_t *t, char c) {
    if (t->mode == RAW)
        return put_std(t->reg_buff, c);

    // dump and flush buffer if registered key is hit
    if (c == t->reg_key && t->reg_buff != NULL) {
        for (int i = 0; i < TERM_BUFFER_SIZE; i++)
            put_std(t->reg_buff, t->in_buff[i]);
        terminal_flush(t);
    }

    // if key pressed is BACKSPACE, delete last char in buffer
    if (c == BACKSPACE) {
        if (t->buff_i > 0) {
            t->buff_i--;
            t->dd->display_putb();
        }

        return 0;
    }

    if (t->buff_i < TERM_BUFFER_SIZE)
        t->in_buff[t->buff_i++] = c;
    
    return 0;
}

/* prints the terminal buffer to the screen 
   eventually want to be able to disable displaying input */
int terminal_display(term_t *t) {
    for (int i = 0; i < t->buff_i && t->in_buff[i] != 0; i++) {
        if (t->in_buff[i] == ESCAPE) {
            i += eval_escape(t, i);
        }

        if (!is_whitespace(t->in_buff[i]))
            t->dd->display_putc(t->in_buff[i]);
    }

    return 0;
}

static int terminal_flush(term_t *t) {
    t->buff_i = 0;
    t->in_buff[0] = 0;
    t->in_buff[1] = 0;

    return 0;
}

/* returns the number of chars to skip in buffer */
static int eval_escape(term_t *t, uint32_t buff_i) {
    buff_i++;
    if (t->in_buff[buff_i] == ESCAPE || t->in_buff[buff_i] == 'M')
        return 2;

    if (t->in_buff[buff_i] == '[') {
        char option = t->in_buff[buff_i + 2]; // get the escape sequence option
        uint32_t display_y = t->dd->display_getcury(t->dd);
        uint32_t display_x  = t->dd->display_getcurx(t->dd);

        switch (option) {
            case 'A':
                int n = t->in_buff[buff_i + 1];
                int new_y = display_y - n;
                new_y = new_y >= 0 ? new_y : 0;
                t->dd->display_setcur(display_x, new_y);
                break;
            case 'B':
                int n = t->in_buff[buff_i + 1];
                int new_y = display_y + n;
                t->dd->display_setcur(display_x, new_y);
                break;
            case 'C':
                int n = t->in_buff[buff_i + 1];
                t->dd->display_setcur(display_x + n, display_y);
                break;
            case 'D':
                int n = t->in_buff[buff_i + 1];
                t->dd->display_setcur(display_x - n, display_y);
                break;
            case 'J': // this isn't techincally correct, should be able to clear portions of screen
                t->dd->display_clear();
                break;
            case ';':
                int m = t->in_buff[buff_i + 1];
                int n = t->in_buff[buff_i + 3];
                t->dd->display_setcur(m,n);
                break;
            default: // either not a valid escape sequence or not supported
                break;
        }

        return 4;
    }

    return 0;
}

static int is_whitespace(char c) {
    if (c <= 31 || c == 127)
        return 1;
    return 0;
}