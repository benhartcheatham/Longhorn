#include "terminal.h"
#include "proc.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../drivers/vga.h"

#define BACKSPACE 0x08

uint16_t last_index = 0;
char *commands[NUM_COMMANDS] = {"shutdown"};
uint32_t terminal_pid;

/* static functions */
static void terminal_waiter(void *aux);
// static void read_stdin();
static void shutdown();

terminal_command command_functions[NUM_COMMANDS] = {shutdown};

void terminal_init() {
    terminal_pid = proc_create("terminal", terminal_waiter, NULL);
    //proc_set_active(terminal_pid);
    //flush_std(stdin);
}

static void terminal_waiter(void *aux __attribute__ ((unused))) {
    while (1) {
        read_stdin();
    }
}

void read_stdin() {

    int i;
    for (i = 0; stdin->stream[i] != '\0'; i++) {
        printf("hi\n");
        if (stdin->stream[i] == '\n') {
            //print the enter
            print("\n");

            //get rid of the enter from the stream
            shrink_std(stdin, 1);
            append_std(stdin, '\0');

            //compare the stream to a command and exectue it if it is one
            int j;
            for (j = 0; j < NUM_COMMANDS; j++)
                if (strcmp(stdin->stream, commands[j]) == 0)
                    command_functions[j](NULL);
            
            //flush stdin and reset index
            flush_std(stdin);
            last_index = 0;
        } else if (stdin->stream[i] == BACKSPACE) {
            //shrink_std(stdin, 1);
            //append_std(stdin, '\0');
            print_backspace();
            last_index--;
        } else {
            vga_print_char(stdin->stream[i]);
            last_index++;
        }
    }

        
}

/* shutsdown the machine gracefully (only works for qemu) */
static void shutdown(void *aux __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}