#include "port_io.h"

void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (val));

}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;

} 

uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}