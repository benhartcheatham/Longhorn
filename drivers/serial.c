
#include <stddef.h>
#include <stdbool.h>
#include "serial.h"
#include "display.h"

#define COM1_LSR (COM1 + 5)
#define SERIAL_TIMEOUT 50000

static void serial_interrupt_handler(struct register_frame *r);
static bool serial_received();
static bool serial_transmit_empty();

void init_serial() {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00);    //                  (hi byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    outb(COM1 + 4, 0x1E);    // Set in loopback mode, test the serial chip
    outb(COM1 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    
    // Check if serial is faulty (i.e: not same byte as sent)
    if(inb(COM1 + 0) != 0xAE) {
       return;
    }
    
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1 + 4, 0x0F);
    register_interrupt_handler(IRQ03, serial_interrupt_handler);
    register_interrupt_handler(IRQ04, serial_interrupt_handler);

    // initialize display driver if we are running testing suite
    #ifdef TESTS
    display_t *dis = get_default_dis_driver();
    dis->dis_putc = serial_putc;
    dis->dis_puts = serial_puts;
    #endif
}

static bool serial_received() {
    return inb(COM1_LSR) & 0x1;
}

static bool serial_transmit_empty() {
    return inb(COM1_LSR) & 0x20;
}

static void serial_interrupt_handler(struct register_frame *r) {
    if (r->int_no != IRQ04)
        return;
    
    
}

int serial_read(uint8_t *buf) {
    if (serial_received()) {
        if (buf != NULL)
            *buf = inb(COM1);
        return 0;
    }

    return -1;
}

int serial_send(uint8_t byte) {
    if (!serial_transmit_empty())
        return -1;
    
    outb(COM1, byte);
    return 0;
}

void serial_putc(char c) {
    int ret = serial_send(c);

    int try_no = 0;
    while (ret == -1 && try_no < SERIAL_TIMEOUT) {
        ret = serial_send(c);
        try_no++;
    }
}

void serial_puts(const char *s) {
    for (int i = 0; s[i] != '\0'; ++i) {
        serial_putc(s[i]);
    }
}


