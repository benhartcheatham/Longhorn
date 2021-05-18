/* Implements the line discipline interface for the default line discipline. This implementation supports 
 * two modes: COOKED and RAW. COOKED mode does all line editing and sends the resulting buffer/character to the
 * output process (if specified). RAW mode sends raw keycodes to the output process, which must be specified in this mode.
 */

/* includes */
#include <string.h>
#include <mem.h>
#include <kerrors.h>
#include "line.h"
#include "../kernel/kalloc.h"

/* defines */

/* globals */
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

/* prototypes */
static int line_in(line_disc_t *ld, char c);
static size_t line_ins(line_disc_t *ld, char *s);
static size_t line_out(line_disc_t *ld);
static size_t line_send(line_disc_t *ld);
static int line_sendv(line_disc_t *ld);
static int line_recv(line_disc_t *ld, char c);
static size_t line_recs(line_disc_t *ld, char *s);
static int line_flush(line_disc_t *ld);
static int line_outbuf(line_disc_t *ld, char *buf);
static int line_outbufn(line_disc_t *ld, char *buf, uint32_t n);

static char eval_kc(term_t *t, unsigned char keycode);
static char kc_getchar(term_t *t, unsigned char c);

/* functions */

/** inits the given line discipline 
 * 
 * @param ld: pointer to the line_discipline struct
 * @param t: if non-NULL, terminal to connect to ld, otherwise a terminal is made
 * @param in: unused
 * @param out: output process to be connected to this ld, must be non-NULL in RAW mode
 * @param m: one of two modes, COOKED or RAW
 * 
 * @return -LINE_INIT_FAIL on failure to initialize, LINE_SUCC otherwise
 */
int line_init(line_disc_t *ld, term_t *t, std_stream *in __attribute__ ((unused)), std_stream *out, ld_modes_t m) {
    if (ld == NULL)
        return -LINE_INIT_FAIL;
    
    ld->line_buffer = kcalloc(LINE_BUFFER_SIZE, sizeof(LINE_BUFFER_SIZE));
    ld->buffer_i = 0;
    if (ld->line_buffer == NULL)
        return -LINE_INIT_FAIL;

    if (t == NULL) {
        ld->term = (term_t *) kmalloc(sizeof(term_t));   // set up terminal
        terminal_init(ld->term, ld, NULL);  // use default display driver
    } else
        ld->term = t;   // assume terminal is already set up

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
    ld->line_outbuf = line_outbuf;
    ld->line_outbufn = line_outbufn;
    
    return LINE_SUCC;
}

/** flushes the line buffer 
 * 
 * @param ld: pointer to line discipline to flush
 * 
 * @return -LINE_FLUSH_FAIL opn failure, LINE_SUCC otherwise
 */
static int line_flush(line_disc_t *ld) {
    if (ld->line_buffer == NULL)
        return -LINE_FLUSH_FAIL;
    
    for (uint32_t i = 0; i < LINE_BUFFER_SIZE; i++)
        ld->line_buffer[i] = 0;

    ld->buffer_i = 0;
    return LINE_SUCC;
}

/** writes a keycode to the line buffer to be output and 
 * sends the corresponding char to the connected process
 *
 * @param ld: line discipline to write to
 * @param c: keycode to write to specified ld
 *
 * @return -LINE_INIT_FAIL on failure, LINE_SUCC otherwise
 */
static int line_in(line_disc_t *ld, char c) {
    // update terminal state
    char kc_ret = eval_kc(ld->term, c);

    if (ld->buffer_i == LINE_BUFFER_SIZE - 1)
        return -LINE_IN_FAIL;
    
    if (ld->mode == RAW) {
        // put char into buffer and send to process
        c = kc_getchar(ld->term, c);
        ld->line_buffer[ld->buffer_i] = c;
        line_sendv(ld);

    } else if (ld->mode == COOKED) {
        if (kc_ret == 0)
            return LINE_SUCC;
        
        if (c == KC_BACKSPACE) {
            if (ld->buffer_i > 0) {
                ld->line_buffer[ld->buffer_i--] = '\0';
                ld->term->dis->dis_backspace();
                ld->term->dis->dis_scur();
            }

            return LINE_SUCC;
        }

        c = kc_getchar(ld->term, c);
        ld->line_buffer[ld->buffer_i] = c;
        line_sendv(ld);
    }

    ld->term->term_write(ld->term, ld->line_buffer[ld->buffer_i++]);
    ld->term->dis->dis_scur();
    return LINE_SUCC;
}

/** writes a string of keycodes to the line buffer to be output and
 * sends the corresponding string to the connected process
 * 
 * @param ld: line discipline to write to
 * @param s: null-terminated string to write
 * 
 * @return number of chars written
 */
static size_t line_ins(line_disc_t *ld, char *s) {
    size_t i;
    for (i = 0; i < strlen(s) && i < LINE_BUFFER_SIZE; i++)
        if (line_in(ld, s[i]) == -LINE_IN_FAIL)
            return i;
    
    return i;
}

/** writes the line buffer to the terminal
 * doesn't flush the line buffer
 * 
 * @param ld: line discipline to send the buffer of
 * 
 * @return number of chars written
 */
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

/** send the entire line_buffer to the connected process
 * doesn't output the buffer to the terminal
 * 
 * @param ld: line discipline to send the buffer of
 * 
 * @return number of characters written
 */
static size_t line_send(line_disc_t *ld) {
    size_t i;
    for (i = 0; i < ld->buffer_i && i < LINE_BUFFER_SIZE && ld->line_buffer[i] != 0; i++)
        put_std(ld->out, ld->line_buffer[i]);

    ld->buffer_i = 0;
    return i;
}

/** send a char from the line_buffer to the connected process
 * doesn't output the character to the terminal 
 * 
 * @param ld: line discipline to send a character from
 * 
 * @return -LINE_OUT_FAIL on failure, LINE_SUCC otherwise 
 */
static int line_sendv(line_disc_t *ld) {
    if (ld->line_buffer[ld->buffer_i] != 0) {
        put_std(ld->out, ld->line_buffer[ld->buffer_i]);
        return LINE_SUCC;
    }

    return -LINE_OUT_FAIL;
}

/** input a char into line buffer from a process
 * char shouldn't be a KC, but an actual character
 * 
 * @param ld: line discipline to write character to
 * @param c: character to write to line discipline, if c is '\b', then 
 *           the last character in the buffer is erased
 * 
 * @return -LINE_IN_FAIL on failure, LINE_SUCC otherwise
 */
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

/** outputs the line buffer to a specified buffer 
 * output buffer should be at least LINE_BUFFER_SIZE chars long
 * 
 * @param ld: line discipline to output from
 * @param buf: buffer to write ld's line buffer into
 * 
 * @return number of characters written
 */
static int line_outbuf(line_disc_t *ld, char *buf) {
    if (buf == NULL)
        return 0;
    
    memcpy(buf, ld->line_buffer, LINE_BUFFER_SIZE);
    return LINE_BUFFER_SIZE;
}

/** outputs the first n chars of the line buffer to a specified buffer 
 * if n is greater than LINE_BUFFER_SIZE, only LINE_BUFFER_SIZE chars are written
 *
 * @param ld: line discipline to output from
 * @param buf: buffer to output ld's line buffer to
 * @param n: number of characters to write to buf
 * 
 * @return number of characters written
 */
static int line_outbufn(line_disc_t *ld, char *buf, uint32_t n) {
    if (buf == NULL)
        return 0;
    
    if (n > LINE_BUFFER_SIZE)
        n = LINE_BUFFER_SIZE;
    
    memcpy(buf, ld->line_buffer, n);

    return n;
}

/** input a string into line buffer from a process
 * char shouldn't be a keycode, but an actual character
 * only inputs the string until the end of the buffer is reached
 * 
 * @param ld: line discipline to write to
 * @param s: null-terminated string to write to ld
 * 
 * @return number of characters written to ld
 */
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

/** returns the default line discipline
 * 
 * @return pointer to the default line discipline
 */
line_disc_t *get_default_line_disc() {
    return &dline;
}

/** utility function that checks if a given keycode is whitespace
 * also updates the state of the specified terminal in some cases
 * 
 * @param t: terminal to get the state from
 * @param keycode: keycode to evaluate
 * 
 * @return 0 if keycode was whitespace, keycode otherwise
 */
static char eval_kc(term_t *t, unsigned char keycode) {
    struct terminal_state *ts = get_term_state(t);

    if (ts == NULL)
        return 0;
    
    switch (keycode) {
        case KC_LALT:
            ts->alt_pressed = !ts->alt_pressed;
            return 0;
        case KC_LSHIFT:
        case KC_RSHIFT:
        case KC_LSHIFT_REL:
        case KC_RSHIFT_REL:
            ts->capitalize = !ts->capitalize;
            return 0;
        case KC_CAPSLOCK:
            ts->capitalize = !ts->capitalize;
            return 0;
        case KC_LCTRL:
        case KC_LCTRL_REL:
            ts->ctrl_pressed = !ts->ctrl_pressed;
            return 0;
        case KC_ESCAPE:
        case KC_NUMLOCK:
        case KC_RELEASED:
            return 0;
        default:
            if (keycode <= KC_MAX)
                return keycode;
            else
                return 0;
    }
}

/** get the ASCII character of a given keycode
 * dependent on the state of the given terminal
 * in the case of unspported keycodes, behavior is undefined
 * 
 * @param t: terminal to get state from
 * @param c: keycode to evaluate
 * 
 * @return ASCII keycode of c, if supported
 */
static char kc_getchar(term_t *t, unsigned char c) {
    struct terminal_state *ts = get_term_state(t);

    if (ts->capitalize == true) 
        return kc_ascii_cap[c];
    else
        return kc_ascii[c];
}