#include <stdbool.h>
#include "terminal.h"
#include "proc.h"
#include "thread.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../drivers/vesa.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"

#define GRAPHICS_MODE 0
#define TEXT_MODE 1

#define NUM_COMMANDS 6
#define NUM_HELP_COMMANDS (NUM_COMMANDS - 2)

size_t last_index = 0;
char *help_commands[NUM_HELP_COMMANDS] = {"help", "shutdown", "ps", "logo"};
char *commands[NUM_COMMANDS] = {"help", "shutdown", "ps", "grub", "moon", "logo"};
uint32_t terminal_pid;

static char key_buffer[TERMINAL_LIMIT + 1];
static uint32_t key_buf_i = 0;
static bool cursor_on = false;

char *full_size_ascii_logo[20] = {
    "dyyyyyhddm                                                           mdhhyyyyyh",
    "     dhysssyhd                                                   dhssssyhd     ",
    "         mhysssyhm                                            mhssssydm        ",
    "            mhssssshm                                      dyssssyd            ",
    "               mhsssssydm      mhhhhhhhhhhhhhhhdm      mhyssssshm              ",
    "                  dysssssssssssssssssssssssssssssssssssssssshm                 ",
    "                     mdysssssssssssssssssssssssssssssssshdm                    ",
    "                     mdysssssssssssssssssssssssssssssssshdm                    ",
    "                    dsssssssssssssssssssssssssssssssssssssym                   ",
    "                       mddm   yssssssssssssssssssh  mmddm                      ",
    "                              dssssssssssssssssssm                             ",
    "                               hssssssssssssssssd                              ",
    "                                hssssssssssssssd                               ",
    "                                 hssssssssssssd                                ",
    "                                  sssssssssssy                                 ",
    "                                  sssssssssssy                                 ",
    "                                 dssssssssssssm                                ",
    "                                 yssssssssssssh                                ",
    "                                  sssssssssssy                                 ",
    "                                   dyyyyyyyyd                                  ",
};

char *half_size_ascii_logo[10] = {
    "ddhhhhm                          mhhhhdm",
    "      dyydm                  mhyyd      ",
    "        mhsshhddhyyyyyyhddhysyh         ",
    "           dyssssssssssssssyd           ",
    "           dhhdhsssssssshdhhd           ",
    "                yssssssy                ",
    "                 yssssy                 ",
    "                 dssssm                 ",
    "                nysssshn                ",
    "                 myssym                 ",
};

/* static functions */
static void terminal_waiter(void *aux);
static void read_stdin();
static void append_to_buffer(char c);
static void shrink_buffer();
static void flush_buffer();

/* command functions */
static void help(void *line);
static void shutdown(void *line);
static void ps(void *line);
static void grub(void *line);
static void moon(void *line);
static void logo(void *line);

terminal_command *command_functions[NUM_COMMANDS] = {help, shutdown, ps, grub, moon, logo};

/* initializes a terminal process */
void terminal_init() {
    terminal_pid = proc_create("terminal", terminal_waiter, NULL);
    proc_set_active(terminal_pid);
    flush_buffer();
}

/* prints the logo of the correpsonding size to the screen
   logo sizes are defined in terminal.h */
void print_logo(int logo_size) {
    int i;

    set_fg_color(0xcc5500);
    
    if (logo_size == FULL_LOGO)
       for (i = 0; i < 20; i++)
            println(full_size_ascii_logo[i]);
    else if (logo_size == HALF_LOGO) 
        for (i = 0; i < 10; i++)
            println(half_size_ascii_logo[i]);
    
    set_fg_color(WHITE);
}

/* function for the terminal process to use, constantly scans input */
static void terminal_waiter(void *aux __attribute__ ((unused))) {
    struct thread *term_thread = thread_get_running();
    uint32_t last_cursor_tick = -1u;

    while (1) {
        read_stdin();
        if (term_thread->ticks % 24 == 0 && term_thread->ticks != last_cursor_tick) {
            if (cursor_on) {
                vesa_hide_cursor();
                cursor_on = false;
            } else {
                vesa_show_cursor();
                cursor_on = true;
            }

            last_cursor_tick = term_thread->ticks;
        }
    }

}

/* reads the active process' stdin stream for input from the user
   input is executed as a command, if available, when the ENTER key is pressed */ 
static void read_stdin() {
    struct process *active = proc_get_active();
    std_stream *stdin = &active->stdin;

    char c = get_std(stdin);
    while (c != -1) {
        if (c == '\n') {
            if (cursor_on)
                vesa_hide_cursor();
            
            vesa_print_char('\n');

            int i;
            for (i = 0; i < NUM_COMMANDS; i++)
                if (strcmp(trim(key_buffer), commands[i]) == 0) {
                    /* this should make a new process/thread, but I need semaphores
                       to ensure the prompt shows up in the right place after it's
                       done */
                    command_functions[i](NULL);
                    break;
                }

            flush_buffer();
            printf("\n> ");
        } else if (c == '\b') {
            if (key_buf_i > 0) {
                //get rid of character the backspace is upposed to get rid of
                if (cursor_on)
                    vesa_hide_cursor();
                
                get_std(stdin);
                shrink_buffer(1);
                vesa_print_backspace();
            }
        } else {
            append_to_buffer(c);
            vesa_print_char(c);

            vesa_show_cursor();
            cursor_on = true;
        }

        c = get_std(stdin);
    }
}

/* adds char c to the key buffer */
static void append_to_buffer(char c) {
    if (key_buf_i < TERMINAL_LIMIT)
        key_buffer[key_buf_i++] = c;
}

/* deletes the last size characters from the buffer
   size must be less than or equal to the current index of the buffer and greater
   than 0 */
static void shrink_buffer() {
    if (key_buf_i > 0)
        key_buffer[--key_buf_i] = '\0';
    else
        key_buffer[key_buf_i] = '\0';
}

/* flushes/clears the key buffer */
static void flush_buffer() {
    key_buf_i = 0;

    int i;
    for (i = 0; i < TERMINAL_LIMIT + 1; i++)
        key_buffer[i] = 0;
}

/* prints a list of available commands */
static void help(void *line __attribute__ ((unused))) {
    printf("Available Commands:\n");
    int i;
    for (i = 0; i < NUM_HELP_COMMANDS; i++) {
        printf("\t%s\n", help_commands[i]);
    }
}
/* shutsdown the machine gracefully (only works for qemu) */
static void shutdown(void *line __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}

static void ps(void *line __attribute__ ((unused))) {
    const list_node *node = proc_peek_all_list();
    
    printf("name");
    print_align("pid", 2);
    print_align("state", 3);
    print_align("active thread\n", 4);
    while (list_hasNext(node)) {
        struct process *proc = (struct process *) node->_struct;
        printf("%s", proc->name);
        print_align(int_to_string(proc->pid), 2);
        print_align(int_to_string(proc->state), 3);
        print_align(proc->active_thread->name, 4);
        printf("\n");

        node = list_get_next(node);
    }

}

/* novelty command */
static void grub(void *line __attribute__ ((unused))) {
    printf("GRUB is ok\n\n\n\ni guess...\n");
}

/* novelty command */
static void moon(void *line __attribute__ ((unused))) {
    printf("did you mean: ");
    set_fg_color(RED);
    
    printf("\"GAMER GOD MOONMOON\"?\n");
    set_default_color();
}

/* prints the OS logo to the screen */
static void logo(void *line __attribute__ ((unused))) {
    print_logo(HALF_LOGO);
}
