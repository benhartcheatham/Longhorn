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
extern void print_align(char *string, uint16_t alignment);

/* shouldn't strictly be in this file 
   but are included for right now */
extern void clear_screen();
extern void copy_screen(char *buffer);
extern uint16_t get_cursor_offset();
extern void set_cursor_offset(uint16_t x, uint16_t y);
extern void set_color(enum vga_color fg, enum vga_color bg);
extern void set_fg_color(enum vga_color c);
extern void set_bg_color(enum vga_color c);
extern void set_default_color();
extern uint16_t get_color();

#endif

