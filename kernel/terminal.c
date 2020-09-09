#include "terminal.h"
#include "proc.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libc/string.h"

/* static functions */
static void shutdown();

void terminal_init() {
    
}

void read_stdin() {
    int command;

    if (strcmp(get_copy_std(stdin), "shutdown") == 0)
        command = 0;
    else
        command = -1;
    
    flush_std(stdin);

    switch (command) {
        case 0:
            shutdown();
            break;
        default:
            return;
    }
        
}

/* shutsdown the machine gracefully (only works for qemu) */
static void shutdown() {
    outw(0x604, 0x2000);
}