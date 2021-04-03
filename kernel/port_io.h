/* Declares functions for doing port IO */
#ifndef PORT_IO_H
#define PORT_IO_H

/* includes */
#include <stdint.h>

/* defines */

/* structs */

/* typedefs */

/* functions */
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);

#endif