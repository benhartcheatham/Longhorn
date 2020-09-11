#include "terminal.h"
#include "proc.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../drivers/vga.h"

#define BACKSPACE 0x0E

uint16_t last_index = 0;
char *commands[NUM_COMMANDS] = {"s"};
uint32_t terminal_pid;

/* static functions */
static void terminal_waiter(void *aux);
static void read_stdin();
static void shutdown();

terminal_command command_functions[NUM_COMMANDS] = {shutdown};

void terminal_init() {
    terminal_pid = proc_create("terminal", terminal_waiter, NULL);
    proc_set_active(terminal_pid);

}

static void terminal_waiter(void *aux __attribute__ ((unused))) {

    //the while loop breaks the keyboard entirely for some reason
    while (1) {
        read_stdin();
    }

}

static void read_stdin() {
    proc_get_active()->stdin.stream[0] = 'a';
    printf("%s\n", proc_get_active()->stdin.stream);
 
}

/* shutsdown the machine gracefully (only works for qemu) */
static void shutdown(void *aux __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}