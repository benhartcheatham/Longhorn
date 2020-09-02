#include "isr.h"
#include "port_io.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "../libc/stdio.h"

/* IDT structs */
/*struct for an IDT interrupt call gate */
struct __attribute__  ((packed)) idt_entry {
    uint16_t offset_low;    //lower bytes of handler address
    uint16_t selector;      //segment selector (kernel code 0x08 for all out interrupts before user mode)
    uint8_t zero;           //zero bits because of how interrupt/trap gate is set up
    uint8_t flags;          //attributes/flags for the gate
    uint16_t offset_high;   //higher bits of handler address
};

/* struct to use for lidt instruction */
struct __attribute__ ((packed)) idt_info {
    uint16_t limit;     //limit has to be 8*N - 1, where N is the amount of IDT entries
    uint32_t addr;      //base address of IDT structure
};

/* IDT data */
struct idt_entry idt[IDT_SIZE];
struct idt_info idt_i;

/* Interrupt Service Rountines data */
char *exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt",  "Breakpoint", "Into Detected Overflow", "Out of Bounds",
    "Invalid Opcode", "Device Not Available", "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS",
    "Segment Not Present", "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved"
};

isr_frame interrupt_handlers[IDT_SIZE];

/* sets up an interrupt vector in the IDT with handler at handler_addr */
void set_idt_gate(uint8_t n, uint32_t handler_addr) {
    idt[n].offset_low = (uint16_t) (handler_addr & 0xFFFF);
    idt[n].selector = 0x08;     //kernel code selector
    idt[n].zero = 0;            //these bits have to be zero because of the way intel set up the gate structure
    idt[n].flags = 0b10001110;  //look to intel manual vol. 3 section 6.11, pg. 2983 for what these bits mean
    idt[n].offset_high = (uint16_t) ((handler_addr >> 16) & 0xFFFF);
}

/* loads a set up IDT into the IDT register */
void load_idt() {
    idt_i.addr = (uint32_t) &idt;
    idt_i.limit = (uint16_t) ((sizeof(struct idt_entry) * IDT_SIZE) - 1);
    asm volatile("lidtl (%0)" : : "r" (&idt_i));
}

/* sets PICs to handle hardware interrupts on vectors 33-47 */
void remap_pics() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0); 
}

/* sets up IA-32 reserved interrupt service routine gates*/
void install_isrs() {
    set_idt_gate(0, (uint32_t) isr00);
    set_idt_gate(1, (uint32_t) isr01);
    set_idt_gate(2, (uint32_t) isr02);
    set_idt_gate(3, (uint32_t) isr03);
    set_idt_gate(4, (uint32_t) isr04);
    set_idt_gate(5, (uint32_t) isr05);
    set_idt_gate(6, (uint32_t) isr06);
    set_idt_gate(7, (uint32_t) isr07);
    set_idt_gate(8, (uint32_t) isr08);
    set_idt_gate(9, (uint32_t) isr09);
    set_idt_gate(10, (uint32_t) isr10);
    set_idt_gate(11, (uint32_t) isr11);
    set_idt_gate(12, (uint32_t) isr12);
    set_idt_gate(13, (uint32_t) isr13);
    set_idt_gate(14, (uint32_t) isr14);
    set_idt_gate(15, (uint32_t) isr15);
    set_idt_gate(16, (uint32_t) isr16);
    set_idt_gate(17, (uint32_t) isr17);
    set_idt_gate(18, (uint32_t) isr18);
    set_idt_gate(19, (uint32_t) isr19);
    set_idt_gate(20, (uint32_t) isr20);
    set_idt_gate(21, (uint32_t) isr21);
    set_idt_gate(22, (uint32_t) isr22);
    set_idt_gate(23, (uint32_t) isr23);
    set_idt_gate(24, (uint32_t) isr24);
    set_idt_gate(25, (uint32_t) isr25);
    set_idt_gate(26, (uint32_t) isr26);
    set_idt_gate(27, (uint32_t) isr27);
    set_idt_gate(28, (uint32_t) isr28);
    set_idt_gate(29, (uint32_t) isr29);
    set_idt_gate(30, (uint32_t) isr30);
    set_idt_gate(31, (uint32_t) isr31);
}

/* sets up hardware interrupt gates */
void install_irqs() {
    set_idt_gate(32, (uint32_t)irq00);
    set_idt_gate(33, (uint32_t)irq01);
    set_idt_gate(34, (uint32_t)irq02);
    set_idt_gate(35, (uint32_t)irq03);
    set_idt_gate(36, (uint32_t)irq04);
    set_idt_gate(37, (uint32_t)irq05);
    set_idt_gate(38, (uint32_t)irq06);
    set_idt_gate(39, (uint32_t)irq07);
    set_idt_gate(40, (uint32_t)irq08);
    set_idt_gate(41, (uint32_t)irq09);
    set_idt_gate(42, (uint32_t)irq10);
    set_idt_gate(43, (uint32_t)irq11);
    set_idt_gate(44, (uint32_t)irq12);
    set_idt_gate(45, (uint32_t)irq13);
    set_idt_gate(46, (uint32_t)irq14);
    set_idt_gate(47, (uint32_t)irq15);
}

/* IA-32 Reserved Interrupt General Handler */
void isr_handler(struct register_frame *r) {
    printf("Recieved Interrupt: %d %s\n", r->int_no, exception_messages[r->int_no]);
    asm volatile("cli");
    asm volatile("hlt");
}

/* Hardware Interrupt General Handler */
void irq_handler(struct register_frame *r) {
    if (interrupt_handlers[r->int_no] != 0) {
        isr_frame handler = interrupt_handlers[r->int_no];
        handler(r);
    }

    if (r->int_no >= 49)
        outb(0xA0, 0x20);   //send an EOI to slave PIC if above IRQ7
    outb(0x20, 0x20);       //send an EOI to master PIC
}

/* Sets a Hardware Interrupt vector to a handler */
void register_interrupt_handler(uint8_t n, isr_frame handler) {
    interrupt_handlers[n] = handler;
}

/* Initializes Hardware Interrupt handlers */
void init_irqs() {
    init_timer(R_FREQ);
    init_keyboard();
}

void init_idt() {
    install_isrs();
    remap_pics();
    install_irqs();
    init_irqs();

    load_idt();
}

void enable_interrupts() {
    asm volatile("sti");
}

void disable_interrupts() {
    asm volatile("cli");
}

