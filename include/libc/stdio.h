/* Defines standard library IO functions */
#ifndef _STDIO_H
#define _STDIO_H

/* includes */
#include <stdint.h>
#include "../../drivers/vga.h"
#include <stream.h>

/* defines */
#define GET_STDIN(p) (&((struct process *) p)->stdin)
#define GET_STDOUT(p) (&((struct process *) p)->stdout)
#define GET_STDERR(p) (&((struct process *) p)->stderr)

#define STDIN (&PROC_CUR()->stdin)
#define STDOUT (&PROC_CUR()->stdout)
#define STDERR (&PROC_CUR()->stderr)

/* structs */
// these should be used to write to the screen, not an active stream
// std_stream *stdin;
// std_stream *stdout;
// std_stream *stderr;

/* typedefs */

/* functions */

/* functions for printing to the screen */
extern void print(char *string);
extern void println(char *string);
extern int printf(const char *string, ...);
extern int sprintf(char *str, const char *format, ...);

#endif

