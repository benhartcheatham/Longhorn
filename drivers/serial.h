

#ifndef SERIAL_H
#define SERIAL_H

/* includes */
#include <stdint.h>
#include "../kernel/isr.h"
#include "../kernel/port_io.h"

/* defines */
#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8 // not reliable?
#define COM4 0x2E8 // not reliable?

/* typedefs */

/* structs */

/* functions */
void init_serial();
int serial_read(uint8_t *buf);
int serial_send(uint8_t byte);

void serial_putc(char c);
void serial_puts(const char *s);

#endif
