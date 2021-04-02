/* Defines functions to set up ISRs and the IDT. */
#ifndef _ISR_H
#define _ISR_H

/* includes */
#include <stddef.h>
#include <stdint.h>

/* defines */
#define IDT_SIZE 256

/* for ease of use accessing hardware interrupts */
#define IRQ00 32
#define IRQ01 33
#define IRQ02 34
#define IRQ03 35
#define IRQ04 36
#define IRQ05 37
#define IRQ06 38
#define IRQ07 39
#define IRQ08 40
#define IRQ09 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

/* prototypes */

/* IA-32 interrupt/exception routines */
extern void isr00();
extern void isr01();
extern void isr02();
extern void isr03();
extern void isr04();
extern void isr05();
extern void isr06();
extern void isr07();
extern void isr08();
extern void isr09();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

/* hardware interrupts (interrupt requests) */
extern void irq00();
extern void irq01();
extern void irq02();
extern void irq03();
extern void irq04();
extern void irq05();
extern void irq06();
extern void irq07();
extern void irq08();
extern void irq09();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* structs */

/* struct for data pushed during an isr */
struct register_frame {
    uint32_t ds;                                            //ds for addressing
    uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax;    //registers saved
    uint32_t int_no, err_code;                              //info for getting right ISR
    uint32_t eip, cs, eflags, esp, ss;                      //pushed by cpu automatically
};

/* typedefs */
typedef void (*isr_frame)(struct register_frame *);

/* functions */
void init_idt();
void install_isrs();
void install_irqs();
void isr_handler(struct register_frame *r);
void irq_handler(struct register_frame *r);
void register_interrupt_handler(uint8_t n, isr_frame handler);
void init_irqs();
void enable_interrupts();
void disable_interrupts();

#endif