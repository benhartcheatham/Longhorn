/* Implementation of the stream data structure. Streams are implemented by a circular queue as of now */

/* includes */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <mem.h>
#include "../kernel/kalloc.h"
#include "stream.h"

/* defines */

/* globals */

/* functions */

/** initializes a char_stream with given size 
 * 
 * @param stream: stream to initialize
 * @param size: size of stream
 * 
 * @return pointer to stream
 */
char_stream *init_c(char_stream *stream, size_t size) {
    stream->stream = (char *) kcalloc(size, sizeof(char));
    stream->size = size;
    stream->in = stream->out = 0;

    return stream;
}

/** flushes char_stream stream
 * 
 * @param stream: stream to flush
 */
void flush_c(char_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->in = stream->out = 0;
}

/** puts char c into char_stream stream
 * 
 * @param stream: stream to input to
 * @param c: character to input
 * 
 * @return 1 on success, 0 otherwise
 */
int put_c(char_stream *stream, char c) {
    if(stream->in == ((stream->out - 1 + stream->size) % stream->size))
        return 0; /* Queue Full*/

    stream->stream[stream->in] = c;

    stream->in = (stream->in + 1) % stream->size;

    return 1;
}

/** returns the address of a char array that has the contents of char_stream stream in it
 * this array needs to be freed with kfree() when done with 
 * 
 * @param stream: stream to copy
 * 
 * @return pointer to copy of stream
 */
char *get_copy_c(char_stream *stream) {
    char *chars = (char *) kcalloc(stream->size, sizeof(char));
    memcpy(chars, stream->stream, stream->size);
    return chars;
}

/** gets and removes the oldest char from char_stream stream
 * 
 * @param stream: stream to get from
 * 
 * @return oldest char in stream
 */
char get_c(char_stream *stream) {
    if(stream->in == stream->out)
        return -1; /* Queue Empty - nothing to get*/

    char old = stream->stream[stream->out];

    stream->out = (stream->out + 1) % stream->size;

    return old;
}

/** resizes the size of a char_stream to size
 * this function destroys the contents of the given stream 
 * 
 * @param stream: stream to resize
 * @param size: new size of stream
 */
void resize_c(char_stream *stream , size_t size) {
    kfree(stream->stream);
    stream = init_c(stream, size);
}

/** frees the underlying char stream within the given char_stream
 * 
 * @param stream: stream to free
 */
void destroy_c(char_stream *stream) {
    kfree(stream->stream);
}

/* std_stream functions */

/** initializes a std_stream with given size 
 * 
 * @param stream: stream to initialize
 * @param size: size of stream
 * 
 * @return pointer to stream
 */
std_stream *init_std(std_stream *stream) {
    stream->size = STD_STREAM_SIZE;
    stream->in = stream->out = 0;

    return stream;
}

/** flushes std_stream stream
 * 
 * @param stream: stream to flush
 */
void flush_std(std_stream *stream) {
    size_t i;
    for (i = 0; i < stream->size; i++)
        stream->stream[i] = 0;
    stream->in = stream->out = 0;
}

/** puts char c into std_stream stream
 * 
 * @param stream: stream to input to
 * @param c: character to input
 * 
 * @return 0 on success, -1 otherwise
 */
int put_std(std_stream *stream, char c) {
    if(stream->in == ((stream->out - 1 + STD_STREAM_SIZE) % STD_STREAM_SIZE))
        return -1; /* Queue Full*/

    stream->stream[stream->in] = c;

    stream->in = (stream->in + 1) % STD_STREAM_SIZE;

    return 0;
}

/** puts string s into std_stream stream
 * 
 * @param stream: stream to input to
 * @param s: string to input
 * 
 * @return number of characters input to stream
 */
int puts_std(std_stream *stream, char *s) {
    size_t len = strlen(s);
    size_t i;
    for (i = 0; i < len && put_std(stream, s[i]) == 0; i++);

    return i;
}

/** returns the address of a char array that has the contents of std_stream stream in it
 * this array needs to be freed with kfree() when done with 
 * 
 * @param stream: stream to copy
 * 
 * @return pointer to copy of stream
 */
char *get_copy_std(std_stream *stream) {
    static char cp[STD_STREAM_SIZE];
    memcpy(cp, stream->stream, STD_STREAM_SIZE);
    return cp;
}

/** gets and removes the oldest char from std_stream stream
 * 
 * @param stream: stream to get from
 * 
 * @return oldest char in stream
 */
char get_std(std_stream *stream) {
    if(stream->in == stream->out)
        return -1; /* Queue Empty - nothing to get*/

    char old = stream->stream[stream->out];

    stream->out = (stream->out + 1) % STD_STREAM_SIZE;

    return old;
}