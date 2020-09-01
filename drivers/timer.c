#include <stdint.h>
#include "timer.h"
#include "../kernel/isr.h"
#include "../kernel/port_io.h"
#include "../libc/stdio.h"

void timer_interrupt_handler(struct register_frame *r);

/* Initializes timer interrupt */
void init_timer(uint32_t freq) {
    /* Install the function we just wrote */
    register_interrupt_handler(IRQ00, timer_interrupt_handler);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)( (divisor >> 8) & 0xFF);
    /* Send the command */
    outb(0x43, 0x36); /* Command port */
    outb(0x40, low);
    outb(0x40, high);
}

/* Interrupt Handler for timer interrupt (IRQ00)
   Should be fast because this code will run a lot */
void timer_interrupt_handler(struct register_frame *r __attribute__ ((unused))) {
    //TODO: Implement with something useful
    println("timer interrupt");
}