#include <stddef.h>
#include <stdint.h>
#include "stream.h"
#include "../libc/mem.h"

void flush_c(char_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->index = 0;
}

void append_c(char_stream *stream, char c) {
    if (stream->index < stream->size) {
        stream->stream[stream->index++] = c;
    }
}

//not implemented until malloc
char *get_copy_c(char_stream *stream __attribute__ ((unused))) {
    return NULL;
}

void shrink_c(char_stream *stream, size_t size) {
    if (size > stream->index)
        size = stream->index;
    
    stream->index = stream->index - size;
}

//not implemented until malloc
void resize_c(char_stream *stream, size_t size) {
    return;
}

/* std_stream functions */
void flush_std(std_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->index = 0;
}

void append_std(std_stream *stream, char c) {
    if (stream->index < stream->size) {
        stream->stream[stream->index++] = c;
    }
}

char *get_copy_std(std_stream *stream) {
    static char cp[STD_STREAM_SIZE];
    memcpy(cp, stream->stream, STD_STREAM_SIZE);
    return cp;
}

void shrink_std(std_stream *stream, size_t size) {
    if (size > stream->index)
        size = stream->index;
    
    stream->index = stream->index - size;
}