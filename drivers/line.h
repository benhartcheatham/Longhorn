#ifndef _LINE_H
#define _LINE_H

#include "terminal.h"
#include <stream.h>

struct line_discipline {
    // included line buffer
    char *line_buffer;

    // connected terminal
    struct terminal *term;

    // connected std_streams
    std_stream *in;
    std_stream *out;

    // initialize the line discipline
    int (*line_init)(struct line_discipline *ld, struct terminal *t, std_stream *in, std_stream *out);

    // recieve input from terminal
    int (*line_in)(struct line_discipline *ld, char c);
    int (*line_ins)(struct line_discipline *ld, char *s);

    // ouput to terminal
    int (*line_out)(struct line_discipline *ld, char c);
    int (*line_outs)(struct line_discipline *ld, char *s);

    // output to other endpoint
    char (*line_send)(struct line_discipline *ld);
    int (*line_sendst)(struct line_discipline *ld, std_stream *s);

    // input from other endpoint
    int (*line_recv)(struct line_discipline *ld, char c);
    int (*line_recs)(struct line_discipline *ld, char *s);
    int (*line_recst)(struct line_discipline *ld, std_stream *s);
};

typedef struct line_discipline line_disc_t;

int line_init(line_disc_t *ld, term_t *t, std_stream *in, std_stream *out);
line_disc_t *get_default_line_disc();
void set_default_line_disc(line_disc_t *ld);
#endif