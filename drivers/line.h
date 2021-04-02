/* Defines the interface for a line discipline as well as some functionality for the default line discipline. */

#ifndef _LINE_H
#define _LINE_H

/* includes */
#include <stream.h>
#include "terminal.h"

/* defines */
#define LINE_BUFFER_SIZE (TERMINAL_BUFF_SIZE + 1)

/* structs */
enum ld_modes {COOKED, RAW};

/* The line discipline does all input formatting and output formatting
 * from/to the terminal */
struct line_discipline {
    /* line buffer state */
    char *line_buffer;
    uint32_t buffer_i;

    /* connected terminal */
    struct terminal *term;

    /* connected std streams */
    std_stream *out;

    /* line buffer mode */
    enum ld_modes mode;

    /** function to initialize the line discipline
     * 
     * @param ld: line discipline to init
     * @param t: terminal to connect to line discipline
     * @param in: input process of the line discipline
     * @param out: output process of the line discipline
     * @param m: mode to set the line discipline to
     * 
     * @return implementation dependent
     */
    int (*line_init)(struct line_discipline *ld, struct terminal *t, std_stream *in, std_stream *out, enum ld_modes m);

    /** flush the line discipline buffer
     *
     * @param ld: line discipline to flush
     * 
     * @return implementation dependent
     */
    int (*line_flush)(struct line_discipline *ld);

    /** recieve a character from the terminal
     *
     * @param ld: line discipline to write to
     * @param c: scancode to write
     * 
     * @return implementation dependent
     */
    int (*line_in)(struct line_discipline *ld, char c);

    /** recieve a null-terminated string from the terminal
     *
     * @param ld: line discipline to write to
     * @param s: null-terminated string of scancodes to write
     * 
     * @return number of characters written
     */
    size_t (*line_ins)(struct line_discipline *ld, char *s);

    /** write to the terminal
     * 
     * @param ld: line discipline to write from
     * 
     * @return number of characters written
     */
    size_t (*line_out)(struct line_discipline *ld);

    /** output ld's line buffer to connected process
     * 
     * @param ld: line discipline to output from
     * 
     * @return number of characters written
     */
    size_t (*line_send)(struct line_discipline *ld);

    /** output the last character in ld's line buffer to connected process
     * 
     * @param ld: line discipline to output from
     * 
     * @return implementation dependent
     */
    int (*line_sendv) (struct line_discipline *ld);

    /** input a chacter into the line_discipline
     * meant to be used from process side of discipline
     * 
     * @param ld: line discipline to write to
     * @param c: character to write
     * 
     * @return implementation dependent
     */
    int (*line_recv)(struct line_discipline *ld, char c);

    /** input a null-terminated string into the line_discipline
     * meant to be used from process side of discipline
     * 
     * @param ld: line discipline to write to
     * @param s: null-terminated string to write
     * 
     * @return number of characters written
     */
    size_t (*line_recs)(struct line_discipline *ld, char *s);

    /** output the line discipline to a buffer
     * 
     * @param ld: line discipline to output
     * @param buf: buffer to output ld to
     * 
     * @return implementation dependent
     */
    int (*line_outbuf)(struct line_discipline *ld, char *buf);

    /** output the line discipline to a buffer
     * 
     * @param ld: line discipline to output
     * @param buf: buffer to output ld to
     * @param n: the number of bytes to write
     * 
     * @return implementation dependent
     */
    int (*line_outbufn)(struct line_discipline *ld, char *buf, uint32_t n);
};

/* typedefs */
typedef struct line_discipline line_disc_t;
typedef enum ld_modes ld_modes_t;

/* functions */
int line_init(line_disc_t *ld, term_t *t, std_stream *in, std_stream *out, ld_modes_t m);
line_disc_t *get_default_line_disc();
#endif