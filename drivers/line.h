#ifndef _LINE_H
#define _LINE_H

#include "terminal.h"
#include <stream.h>

#define LINE_BUFFER_SIZE (TERMINAL_BUFF_SIZE + 1)

enum ld_modes {COOKED, RAW};

/* The line discipline does all input formatting and output formatting
 * from/to the terminal. The line discipline has one specified process 
 * output (the out std_stream) to which the line discipline will relay
 * input. COOKED mode converts the keycodes and sanitizes input, while 
 * RAW mode just converts keycodes. */
struct line_discipline {
    // included line buffer
    char *line_buffer;
    uint32_t buffer_i;

    // connected terminal
    struct terminal *term;

    // connected std_streams
    std_stream *out;

    // line buffer mode
    enum ld_modes mode;

    // initialize the line discipline
    int (*line_init)(struct line_discipline *ld, struct terminal *t, std_stream *in, std_stream *out);

    // flush the line discipline buffer
    int (*line_flush)(struct line_discipline *ld);

    // recieve input from terminal
    int (*line_in)(struct line_discipline *ld, char c);
    size_t (*line_ins)(struct line_discipline *ld, char *s);

    // write to terminal
    size_t (*line_out)(struct line_discipline *ld);

    // ouput to terminal from process
    // these aren't needed currently, but the would be faster
    //int (*line_proc_in)(struct line_discipline *ld, char c);
    //int (*line_proc_ins)(struct line_discipline *ld, char *s);

    // output to std_out, doesn't send to terminal
    size_t (*line_send)(struct line_discipline *ld);
    int (*line_sendv) (struct line_discipline *ld);

    // input from process, doesn't send to terminal
    int (*line_recv)(struct line_discipline *ld, char c);
    size_t (*line_recs)(struct line_discipline *ld, char *s);
};

typedef struct line_discipline line_disc_t;
typedef enum ld_modes ld_modes_t;

int line_init(line_disc_t *ld, std_stream *out, ld_modes_t m);
line_disc_t *get_default_line_disc();
#endif