#include <stddef.h>
#include <stdint.h>
#include "stream.h"
#include "../kernel/kalloc.h"
#include "../libc/mem.h"


char_stream *init_c(char_stream *stream, size_t size) {
    stream = (char *) kcalloc(size, sizeof(char));
    stream->size = size;
    stream->index = 0;
}

void flush_c(char_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->index = 0;
}

int append_c(char_stream *stream, char c) {
    if (stream->index < stream->size) {
        stream->stream[stream->index++] = c;
        return 1;
    }

    return 0;
}

//not implemented until malloc
char *get_copy_c(char_stream *stream __attribute__ ((unused))) {
    return NULL;
}

int shrink_c(char_stream *stream, size_t size) {
    if (size > stream->index)
        size = stream->index;
    
    stream->index = stream->index - size;

    return size;
}

//not implemented until malloc
void resize_c(char_stream *stream __attribute__ ((unused)), size_t size __attribute__ ((unused))) {
    return;
}

/* std_stream functions */

std_stream *init_std(std_stream *stream) {
    stream->size = STD_STREAM_SIZE;
    stream->index = 0;
}

void flush_std(std_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->index = 0;
}

int append_std(std_stream *stream, char c) {
    if (stream->index < stream->size) {
        stream->stream[stream->index] = c;
        stream->index++;
        return 1;
    }

    return 0;
}

char *get_copy_std(std_stream *stream) {
    static char cp[STD_STREAM_SIZE];
    memcpy(cp, stream->stream, STD_STREAM_SIZE);
    return cp;
}

int shrink_std(std_stream *stream, size_t size) {
    if (size > stream->index)
        size = stream->index;
    
    stream->index = stream->index - size;
    
    return size;
}