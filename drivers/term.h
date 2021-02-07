#ifndef _TERMINAL_H
#define _TERMINAL_H

/* Right now you have to call putc to do anything, but this might want to
 * process later on
 */

/* includes */
#include <stdint.h>
#include <stream.h>
#include "keyboard.h"
#include "display.h"
#include "../kernel/proc.h"

/* defines */
#define TERM_BUFFER_SIZE STD_STREAM_SIZE
#define ESCAPE 27 // ASCII code for the ESCAPE key

/* structs */

struct terminal {
    char in_buff[TERM_BUFFER_SIZE + 7]; // extra apace if for null terminator and escape sequences
    uint32_t buff_i;

    char reg_key;
    std_stream *reg_buff;

    key_modes_t mode;
    key_driver_t *kd;
    dis_driver_t *dd;

    struct terminal (*terminal_init) (key_modes_t mode, std_stream *in, key_driver_t *kd, dis_driver_t *dd, void *aux);
    int (*terminal_mode) (struct terminal *t, key_modes_t mode);
    int (*terminal_register) (struct terminal *t, std_stream *in, char c); // registers the key press which sends the buffer to
                                                                            // the calling program when in COOKED mode
    char (*terminal_getc) (void);   // get input from terminal
    int (*terminal_writec) (struct terminal *t, char c);    // terminal display function
    int (*terminal_display) (struct terminal *t);
};

/* typedefs */
typedef struct terminal term_t;

/* functions */

/* our driver functions */
term_t terminal_init(key_modes_t mode, std_stream *in, key_driver_t *kd, dis_driver_t *dd, void *aux);
int terminal_mode(term_t *t, key_modes_t mode);
char terminal_getc();
int terminal_writec(term_t *t, char c);

/* getter functions */
term_t get_terminal();
#endif