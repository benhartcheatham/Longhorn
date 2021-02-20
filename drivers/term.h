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

/* structs */

struct terminal {
    char in_buff[TERM_BUFFER_SIZE + 7]; // extra space if for null terminator and escape sequences
    uint32_t buff_i;

    char reg_key;
    std_stream *reg_buff;

    key_modes_t mode;
    key_driver_t *kd;
    dis_driver_t *dd;

    int (*init) (struct terminal *t, key_modes_t mode, std_stream *in, key_driver_t *kd, dis_driver_t *dd, void *aux);
    int (*set_mode) (struct terminal *t, key_modes_t mode);
    int (*registerk) (struct terminal *t, std_stream *in, char c); // registers the key press which sends the buffer to
                                                                           // the calling program when in COOKED mode
    char (*getc) (struct terminal *t);   // get input from terminal
    int (*writec) (struct terminal *t, char c);    // write input to the terminal
    int (*display) (struct terminal *t);   // display terminal buffer
    int (*flush) (struct terminal *t); // flush the terminal buffer
};

/* typedefs */
typedef struct terminal term_t;

/* functions */

/* our driver functions */
int terminal_init(term_t *t, key_modes_t mode, std_stream *in, key_driver_t *kd, dis_driver_t *dd, void *aux);

/* getter functions */
term_t get_terminal();
#endif