/* Implements functionality for doing port IO */

/* includes */
#include "port_io.h"

/* defines */

/* globals */

/* functions */

/** writes a byte to a port
 * 
 * @param port: port to write to
 * @param val: byte to write to port
 */
void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %1, %0" : : "dN" (port), "a" (val));

}

/** gets a byte from a port
 * 
 * @param port: port to read form
 * 
 * @return byte returned from read port
 */
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;

} 

/** writes a word (2 bytes) to a port
 * 
 * @param port: port to write to
 * @param val: word to write to port
 */
void outw(uint16_t port, uint16_t val) {
    asm volatile("out %%ax, %%dx" : : "a" (val), "d" (port));
}


/** gets a word (2 bytes) from a port
 * 
 * @param port: port to read form
 * 
 * @return word returned from read port
 */
uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}