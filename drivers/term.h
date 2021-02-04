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

/* structs */

struct terminal {
    key_modes_t mode;
    key_driver_t *kd;
    dis_driver_t *dd;
    std_stream *in;
    std_stream *out;
    struct terminal (*terminal_init) (std_stream *in, std_stream *out, key_driver_t *kd, dis_driver_t *dd, void *aux);
    int (*terminal_din) (struct terminal *t, std_stream *in);
    int (*terminal_dout) (struct terminal *t, std_stream *out);
    int (*terminal_mode) (struct terminal *t, key_modes_t mode);
    int (*terminal_putc) (char c);
};

/* typedefs */
typedef struct terminal term_t;

/* functions */

/* our driver functions */
term_t terminal_init(std_stream *in, std_stream *out, key_driver_t *kd, dis_driver_t *dd, void *aux);
int terminal_mode(term_t *t, key_modes_t mode);
int terminal_in(term_t *t, std_stream *in);
int terminal_out(term_t *t, std_stream *out);
int terminal_putc(char c);

/* getter functions */
term_t get_terminal();
#endif