#include <kerrors.h>
#include <mem.h>
#include <string.h>
#include "line.h"
#include "../kernel/kalloc.h"

static struct line_discipline *dline;

static int line_in(line_disc_t *ld, char c);
static int line_ins(line_disc_t *ld, char *s);
static int line_out(line_disc_t *ld, char c);
static int line_outs(line_disc_t *ld, char *s);
static int line_send(line_disc_t *ld);
static int line_sendst(line_disc_t *ld, std_stream *s);
static int line_recv(line_disc_t *ld, char c);
static int line_recs(line_disc_t *ld, char *s);
static int line_recst(line_disc_t *ld, std_stream *s);

static int line_init(line_disc_t *ld, term_t *t, std_stream *in, std_stream *out) {
    if (ld == NULL || t == NULL)
        return -LINE_INIT_FAIL;
    
    ld->line_buffer = kcalloc(TERMINAL_BUFF_SIZE + 1, sizeof(TERMINAL_BUFF_TYPE));
    if (ld->line_buffer == NULL)
        return -LINE_INIT_FAIL;
    
    ld->term = t;

    if (in)
        ld->in = in;
    
    if (out)
        ld->out = out;
    
    ld->line_init = NULL;
    ld->line_in = line_in; // unimplemented
    ld->line_ins = line_ins;
    ld->line_out = line_out; // unimplemented
    ld->line_outs = line_outs;
    ld->line_send = line_send;
    ld->line_sendst = line_sendst;
    ld->line_recv = line_recv;  // unimplemented
    ld->line_recs = line_recs;
    ld->line_recst = line_recst;

    return LINE_SUCC;
}

static int line_in(line_disc_t *ld, char c) {
    return -LINE_IN_FAIL;
}

static int line_ins(line_disc_t *ld, char *s) {
    memcpy(ld->in, s, strlen(s));
    return LINE_SUCC;
}

static int line_out(line_disc_t *ld, char c) {
    return -LINE_OUT_FAIL;
}

static int line_outs(line_disc_t *ld, char *s) {
    return ld->term->term_ins(ld->term, s);
}

static int line_send(line_disc_t *ld) {
    size_t s_size = strlen(ld->line_buffer);

    size_t i;
    for (i = 0; i < s_size && i < TERMINAL_BUFF_SIZE; i++)
        put_std(ld->out, ld->line_buffer[i]);
    
    ld->line_buffer[i] = 0;
    return LINE_SUCC;
}

static int line_sendst(line_disc_t *ld, std_stream *s) {
    size_t s_size = strlen(ld->line_buffer);

    size_t i;
    for (i = 0; i < s_size && i < TERMINAL_BUFF_SIZE; i++)
        put_std(s, ld->line_buffer[i]);
    
    ld->line_buffer[i] = 0;
    return LINE_SUCC;
}

static int line_recv(line_disc_t *ld, char c) {
    return -LINE_IN_FAIL;
}

static int line_recs(line_disc_t *ld, char *s) {
    size_t s_size = strlen(s);
    s_size = s_size < TERMINAL_BUFF_SIZE ? s_size : TERMINAL_BUFF_SIZE;
    memcpy(ld->line_buffer, s, s_size);
    ld->line_buffer[s_size] = 0;
    return s_size;
}

static int line_recst(line_disc_t *ld, std_stream *s) {
    int i = 0;
    char c = get_std(s);

    while (c != -1 && i < TERMINAL_BUFF_SIZE) {
        ld->line_buffer[i] = c;
        i++;
    }

    ld->line_buffer[i] = 0;
    return i;
}

line_disc_t *get_default_line_disc() {
    return dline;
}

void set_default_line_disc(line_disc_t *ld) {
    dline = ld;
}