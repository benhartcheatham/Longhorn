/* Streams are implemented by a circular queue as of now */
#include <stddef.h>
#include <stdint.h>
#include "stream.h"
#include "../kernel/kalloc.h"
#include "../libc/mem.h"


char_stream *init_c(char_stream *stream, size_t size) {
    stream->stream = (char *) kcalloc(size, sizeof(char));
    stream->size = size;
    stream->in = stream->out = 0;

    return stream;
}

void flush_c(char_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->in = stream->out = 0;
}

int put_c(char_stream *stream, char c) {
    if(stream->in == ((stream->out - 1 + stream->size) % stream->size))
        return 0; /* Queue Full*/

    stream->stream[stream->in] = c;

    stream->in = (stream->in + 1) % stream->size;

    return 1;
}

//not implemented until malloc
char *get_copy_c(char_stream *stream __attribute__ ((unused))) {
    return NULL;
}

char get_c(char_stream *stream) {
    if(stream->in == stream->out)
        return -1; /* Queue Empty - nothing to get*/

    char old = stream->stream[stream->out];

    stream->out = (stream->out + 1) % stream->size;

    return old;
}

//not implemented until malloc
void resize_c(char_stream *stream __attribute__ ((unused)), size_t size __attribute__ ((unused))) {
    return;
}

/* std_stream functions */

std_stream *init_std(std_stream *stream) {
    stream->size = STD_STREAM_SIZE;
    stream->in = stream->out = 0;

    return stream;
}

void flush_std(std_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->in = stream->out = 0;
}

int put_std(std_stream *stream, char c) {
    if(stream->in == ((stream->out - 1 + STD_STREAM_SIZE) % STD_STREAM_SIZE))
        return 0; /* Queue Full*/

    stream->stream[stream->in] = c;

    stream->in = (stream->in + 1) % STD_STREAM_SIZE;

    return 1;
}

char *get_copy_std(std_stream *stream) {
    static char cp[STD_STREAM_SIZE];
    memcpy(cp, stream->stream, STD_STREAM_SIZE);
    return cp;
}

char get_std(std_stream *stream) {
    if(stream->in == stream->out)
        return -1; /* Queue Empty - nothing to get*/

    char old = stream->stream[stream->out];

    stream->out = (stream->out + 1) % STD_STREAM_SIZE;

    return old;
}