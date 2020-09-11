#include "terminal.h"
#include "proc.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../drivers/vga.h"

#define BACKSPACE 0x0E
#define ENTER '\n'

uint16_t last_index = 0;
char *commands[NUM_COMMANDS] = {"shutdown"};
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
    if (last_index == get_index_std(&proc_get_active()->stdin))
        return;
    
    char *stdin_cp = proc_get_in(proc_get_active());

    /* This implementation is prone to errors but gets the job done for now.
     * Specifically, it assumes that the enter is the last character in the stream
     * characters after the ENTER are truncated and lost. Backspaces may also delete
     * the wrong character if backspace is pressed and then a character is input
     * before this function finishes. Come back to this to iron this stuff out
     */
    int i;
    for (i = last_index; i < STD_STREAM_SIZE && stdin_cp[i] != '\0'; i++) {
        if (stdin_cp[i] == ENTER) {
            //print the enter
            print("\n");

            //remove the enter from the string and replace with a terminator
            stdin_cp[i] = '\0';
            //find command and execute it if it exists
            int j;
            for (j = 0; j < NUM_COMMANDS; j++)
                if (strcmp(stdin_cp, commands[j]) == 0)
                    command_functions[j](NULL);
            
            flush_std(&proc_get_active()->stdin);
        } else if (stdin_cp[i] == BACKSPACE) {
            shrink_std(&proc_get_active()->stdin, 1);
            append_std(&proc_get_active()->stdin, '\0');
            print_backspace();
        } else {
            vga_print_char(stdin_cp[i]);
        }

        last_index = get_index_std(&proc_get_active()->stdin);
    }
}

/* shutsdown the machine gracefully (only works for qemu) */
static void shutdown(void *aux __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}