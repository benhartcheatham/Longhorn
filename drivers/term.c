/* Terminal is only meant for input from the keyboard currently,
 * this should be expanded to be any hardware device later on.
 * There can also only be one terminal on the machine at once,
 * this should probably be changed. */

#include "term.h"
#include "vesa.h"
#include "vga.h"
#include <stdio.h>

// static const char kc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
//                           '7', '8', '9', '0', '-', '=', '?', '?', 
//                           'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
//                           'o', 'p', '[', ']', '\n', '?', 'a', 's', 
//                           'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
//                           '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
//                           'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

// static const char kc_ascii_cap[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
//     '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
//         'U', 'I', 'O', 'P', '[', ']', '\n', '?', 'A', 'S', 'D', 'F', 'G', 
//         'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
//         'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

static term_t *default_term;

static int terminal_mode(term_t *t, term_modes_t mode);
static int terminal_register(term_t *t, std_stream *in, char c);
static char terminal_getc(term_t *t);
static int terminal_writec(term_t *t, char c);
static int terminal_display(term_t *t);
static int terminal_flush(term_t *t);
static int eval_escape(term_t *t, uint32_t buff_i);
static int is_whitespace(char c);

int terminal_init(term_t *t, term_modes_t mode, std_stream *in, key_driver_t *kd, dis_driver_t *dd, void *aux __attribute__ ((unused))) {
    t->init = NULL;
    t->set_mode = terminal_mode;
    t->registerk = terminal_register;
    t->getc = terminal_getc;
    t->writec = terminal_writec;
    t->display = terminal_display;
    t->flush = terminal_flush;

    t->flush(t);
    t->kd = kd;
    t->dd = dd;
    t->set_mode(t, mode);
    t->buff_i = 0;
    
    // a buffer must be supplied for RAW mode, otherwise there is
    // undefined behavior
    if (in != NULL && mode == TRAW)
        t->reg_buff = in;
    else
        t->reg_buff = NULL;
    
    t->reg_key = 0;
    t->kd->set_mode(mode);
    
    if (default_term == NULL)
        default_term = t;
    
    return 0;
}

/* sets the mode of the terminal
   RAW does no input filtering from the keyboard driver and passes it on
   to the std process
   COOKED does filtering with some other extra options */
static int terminal_mode(term_t *t, term_modes_t mode) {
    if (t) {
        t->mode = mode;
        t->kd->set_mode(mode);
    } else
        return -1;
    
    return 0;
}

/* registers a key that when pressed dumps the entire terminal buffer into a 
   specified buffer 
   only available in COOKED mode */
static int terminal_register(term_t *t, std_stream *in, char c) {
    // this feature is only available in COOKED mode
    if (t->mode != TCOOKED)
        return -1;
    
    t->reg_key = c;
    t->reg_buff = in;
    return 0;
}

/* returns the lastest character in the terminal buffer and deletes it from the buffer
   if it exists
   if the char doesn't exists, returns -1 */
static char terminal_getc(term_t *t) {
    char c = t->in_buff[t->buff_i];

    if (t->buff_i > 0) {
        t->buff_i--;
        return c;
    }

    return -1;
}

/* writes a char to the terminal buffer, with a few exceptions.
   if the terminal is in raw mode, the char is written directly to the
   stdin of the process and this function returns
   if a key has been registered, the terminal buffer is dumped to the registered buffer 
   and flushed and this function returns
   the input is then checked for special characters and if it is a regular character,
   it is put in the terminal buffer */
   
static int terminal_writec(term_t *t, char c) {

    if (t->mode == TRAW)
        return put_std(t->reg_buff, c);

    // dump and flush buffer if registered key is hit
    if (c == t->reg_key && t->reg_buff != NULL) {
        for (int i = 0; i < TERM_BUFFER_SIZE; i++)
            puts_std(t->reg_buff, t->in_buff);
        terminal_flush(t);

        return 0;
    }

    // if key pressed is BACKSPACE, delete last char in buffer
    if (c == ASCII_BACKSPACE) {
        if (t->buff_i > 0) {
            t->buff_i--;
            t->dd->backspace();
        }

        return 0;
    }

    // should probably check for actual escape codes, but that require extra
    // state to do so that i am not sure i want
    if (t->buff_i < TERM_BUFFER_SIZE)
        t->in_buff[t->buff_i++] = c;
    
    t->dd->putc(c);
    return 0;
}

/* prints the terminal buffer to the screen 
   eventually want to be able to disable displaying input */
static int terminal_display(term_t *t) {
    for (uint32_t i = 0; i < t->buff_i && t->in_buff[i] != 0; i++) {
        if (t->in_buff[i] == ASCII_ESCAPE) {
            i += eval_escape(t, i);
        }

        t->dd->putc(t->in_buff[i]);
    }

    terminal_flush(t);
    return 0;
}

/* sets the whole buffer to 0 and resets the buffer index */
static int terminal_flush(term_t *t) {
    t->buff_i = 0;
    
    for (int i = 0; i < TERM_BUFFER_SIZE; i++)
        t->in_buff[i] = 0;

    return 0;
}

/* returns the number of chars to skip in buffer */
static int eval_escape(term_t *t, uint32_t buff_i) {
    buff_i++;
    if (t->in_buff[buff_i] == ASCII_ESCAPE || t->in_buff[buff_i] == 'M')
        return 2;

    if (t->in_buff[buff_i] == '[') {
        char option = t->in_buff[buff_i + 2]; // get the escape sequence option
        uint32_t display_y = t->dd->getcury();
        uint32_t display_x  = t->dd->getcurx();

        int n = 0;
        int m = 0;
        int new_y = 0;
        switch (option) {
            case 'A':
                n = t->in_buff[buff_i + 1];
                new_y = display_y - n;
                new_y = new_y >= 0 ? new_y : 0;
                t->dd->setcur(display_x, new_y);
                break;
            case 'B':
                n = t->in_buff[buff_i + 1];
                new_y = display_y + n;
                t->dd->setcur(display_x, new_y);
                break;
            case 'C':
                n = t->in_buff[buff_i + 1];
                t->dd->setcur(display_x + n, display_y);
                break;
            case 'D':
                n = t->in_buff[buff_i + 1];
                t->dd->setcur(display_x - n, display_y);
                break;
            case 'J': // this isn't techincally correct, should be able to clear portions of screen
                t->dd->clear();
                break;
            case 'P':
                n = t->in_buff[buff_i + 1];
                for (int i = 0; i < n; i++)
                    t->writec(t, ASCII_BACKSPACE);
                break;
            case ';':
                m = t->in_buff[buff_i + 1];
                n = t->in_buff[buff_i + 3];
                t->dd->setcur(m,n);
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