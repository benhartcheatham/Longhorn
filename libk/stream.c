/* Streams are implemented by a circular queue as of now */
#include <stddef.h>
#include <stdint.h>
#include "stream.h"
#include "../kernel/kalloc.h"
#include "../libc/mem.h"

/* initializes a char_stream with given size */
char_stream *init_c(char_stream *stream, size_t size) {
    stream->stream = (char *) kcalloc(size, sizeof(char));
    stream->size = size;
    stream->in = stream->out = 0;

    return stream;
}

/* flushes the given char_stream */
void flush_c(char_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->in = stream->out = 0;
}

/* puts the given char into the given char_stream */
int put_c(char_stream *stream, char c) {
    if(stream->in == ((stream->out - 1 + stream->size) % stream->size))
        return 0; /* Queue Full*/

    stream->stream[stream->in] = c;

    stream->in = (stream->in + 1) % stream->size;

    return 1;
}

/* returns the address of a char array that has the contents of the given char_stream in it
   this array needs to be free'd with kfree() when done with */
char *get_copy_c(char_stream *stream) {
    char *chars = (char *) kcalloc(stream->size, sizeof(char));
    memcpy(chars, stream->stream, stream->size);
    return chars;
}

/* gets and removes the oldest char from the char_stream */
char get_c(char_stream *stream) {
    if(stream->in == stream->out)
        return -1; /* Queue Empty - nothing to get*/

    char old = stream->stream[stream->out];

    stream->out = (stream->out + 1) % stream->size;

    return old;
}

/* resizes the size of a char_stream to size
   this function destroys the contents of the given stream */
void resize_c(char_stream *stream , size_t size) {
    kfree(stream->stream);
    stream = init_c(stream, size);
}

/* frees the underlying char stream within the given char_stream
   if the char_stream was obtained with malloc,
   the char_stream must also be freed explicitly. */
void destroy_c(char_stream *stream) {
    kfree(stream->stream);
}

/* std_stream functions */

/* initializes the given std_stream */
std_stream *init_std(std_stream *stream) {
    stream->size = STD_STREAM_SIZE;
    stream->in = stream->out = 0;

    return stream;
}

/* flushes the given std_stream */
void flush_std(std_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->in = stream->out = 0;
}

/* puts the given char into the given std_stream */
int put_std(std_stream *stream, char c) {
    if(stream->in == ((stream->out - 1 + STD_STREAM_SIZE) % STD_STREAM_SIZE))
        return 0; /* Queue Full*/

    stream->stream[stream->in] = c;

    stream->in = (stream->in + 1) % STD_STREAM_SIZE;

    return 1;
}

/* returns the address of a char array that has the contents of the given char_stream in it */
char *get_copy_std(std_stream *stream) {
    static char cp[STD_STREAM_SIZE];
    memcpy(cp, stream->stream, STD_STREAM_SIZE);
    return cp;
}

/* retusn and removes the oldest char from the stream */
char get_std(std_stream *stream) {
    if(stream->in == stream->out)
        return -1; /* Queue Empty - nothing to get*/

    char old = stream->stream[stream->out];

    stream->out = (stream->out + 1) % STD_STREAM_SIZE;

    return old;
}