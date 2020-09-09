#ifndef _STREAM_H
#define _STREAM_H

/* includes */
#include <stddef.h>

/* defines */
#define STD_STREAM_SIZE 256

/* structs */
struct CHAR_STREAM {
    char *stream;
    size_t size;
    size_t index;
};

struct STD_STREAM {
    char stream[STD_STREAM_SIZE];
    size_t size;
    size_t index;
};

/* typedefs */
typedef struct STD_STREAM std_stream;
typedef struct CHAR_STREAM char_stream;

/* functions */

/* char_stream functions */
char_stream *init_c(char_stream *stream, size_t size);
void flush_c(char_stream *stream);
int append_c(char_stream *stream, char c);
char *get_copy_c(char_stream *stream);
int shrink_c(char_stream *stream, size_t size);
void resize_c(char_stream *stream, size_t size);

/* std_stream functions */
std_stream *init_std(std_stream *stream);
void flush_std(std_stream *stream);
int append_std(std_stream *stream, char c);
char *get_copy_std(std_stream *stream);
int shrink_std(std_stream *stream, size_t size);

#endif