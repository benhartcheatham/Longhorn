#include <stdbool.h>
#include "shell.h"
#include "thread.h"
#include "port_io.h"
#include "proc.h"
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include "../drivers/terminal.h"
#include "../drivers/bmp.h"

#define GRAPHICS_MODE 0
#define TEXT_MODE 1

#define NUM_COMMANDS 6
#define NUM_HELP_COMMANDS (NUM_COMMANDS - 2)

#define LOGO_COLOR 0xBD5615

/* shell info */
size_t last_index = 0;
char *help_commands[NUM_HELP_COMMANDS] = {"help", "shutdown", "exit", "ps"};
char *commands[NUM_COMMANDS] = {"help", "shutdown", "exit", "ps", "grub", "moon"};
uint32_t shell_pid;

/* logo variables */
extern char _binary_assets_longhorn_logo_bmp_start;
uint8_t *header_addr = (uint8_t *) &_binary_assets_longhorn_logo_bmp_start;
bmp_file_header_t header;

/* key buffer info */
static char key_buffer[LINE_BUFFER_SIZE];
//static uint32_t key_buf_i = 0;
static bool cursor_on = false;

/* static functions */
static void shell_waiter(void *aux);
static void read_stdin(struct process *active);
//static void append_to_buffer(char c);
//static void shrink_buffer();
//static void flush_buffer();

/* command functions */
static void help(void *line);
static void shutdown(void *line);
static void ps(void *line);
static void grub(void *line);
static void moon(void *line);
shell_command *command_functions[NUM_COMMANDS] = {help, shutdown, shutdown, ps, grub, moon};

/* initializes a shell process */
void shell_init() {
    shell_pid = proc_create("shell", shell_waiter, NULL);
    proc_set_active(shell_pid);
    //flush_buffer();

    read_bmp_header(header_addr, &header);
    bmp_change_color(&header, 0xFFFFFF, 0x0);
}

/* prints the logo of the correpsonding size to the screen
   logo sizes are defined in terminal.h */
void print_logo() {
    clear_screen();
    draw_bmp_data(&header, 10, vesa_get_cursor_y() * FONT_HEIGHT + 10);
    vesa_set_cursor(0, (header.info_header.height / FONT_HEIGHT) + 1);
}

/* function for the shell process to use, constantly scans input */
static void shell_waiter(void *aux __attribute__ ((unused))) {
    uint32_t last_cursor_tick = 0;

    while (1) {
        if (THREAD_CUR()->ticks % 48 == 0 && THREAD_CUR()->ticks != last_cursor_tick) {
            if (cursor_on) {
                get_default_dis_driver()->dis_hcur();
                cursor_on = false;
            } else {
                get_default_dis_driver()->dis_scur();
                cursor_on = true;
            }

            last_cursor_tick = THREAD_CUR()->ticks;
        }

        if (proc_get_active()->pid == shell_pid)
            read_stdin(proc_get_active());

    }

}

/* reads the active process' stdin stream for input from the user
   input is executed as a command, if available, when the ENTER key is pressed */ 
static void read_stdin(struct process *active) {
    line_disc_t *ld = get_default_line_disc();
    std_stream *stdin = GET_STDIN(active);
    display_t *dis = get_default_dis_driver();

    char c = get_std(stdin);
    //printf("hello %d\n", THREAD_CUR()->ticks);
    while (c != -1) {
        printf("fuck\n");
        if (c == '\n') {
            dis->dis_hcur();
            ld->line_send(ld);
            memcpy(key_buffer, stdin->stream, STD_STREAM_SIZE);

            int i;
            for (i = 0; i < NUM_COMMANDS; i++)
                if (strcmp(trim(key_buffer), commands[i]) == 0) {
                    /* this should make a new process/thread, but I need to be able
                    *  to wait on child processes  */

                    command_functions[i](NULL);
                    break;
                }

            flush_std(stdin);
            ld->line_flush(ld);
            printf("\n> ");
        }
        
        c = get_std(stdin);
    }
}

// /* adds char c to the key buffer */
// static void append_to_buffer(char c) {
//     if (key_buf_i < LINE_BUFFER_SIZE)
//         key_buffer[key_buf_i++] = c;
// }

// /* deletes the last size characters from the buffer
//    size must be less than or equal to the current index of the buffer and greater
//    than 0 */
// static void shrink_buffer() {
//     if (key_buf_i > 0)
//         key_buffer[--key_buf_i] = '\0';
//     else
//         key_buffer[key_buf_i] = '\0';
// }

// /* flushes/clears the key buffer */
// static void flush_buffer() {
//     key_buf_i = 0;

//     int i;
//     for (i = 0; i < LINE_BUFFER_SIZE + 1; i++)
//         key_buffer[i] = 0;
// }

/* prints a list of available commands */
static void help(void *line __attribute__ ((unused))) {
    printf("Available Commands:\n");
    int i;
    for (i = 0; i < NUM_HELP_COMMANDS; i++) {
        printf("\t%s\n", help_commands[i]);
    }
}
/* shuts down the machine gracefully (only works for qemu) */
static void shutdown(void *line __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}


static char *p_state_to_string(enum thread_states s) {
    switch(s) {
        case THREAD_RUNNING:
            return "RUN";
        case THREAD_READY:
            return "REA";
        case THREAD_DYING:
            return "DYI";
        case THREAD_TERMINATED:
            return "TER";
        case THREAD_BLOCKED:
            return "BLO";
        default:
            return "UND";

    }
}

static void ps(void *line __attribute__ ((unused))) {
    const list_node *node = proc_peek_all_list();
    
    printf("name");
    print_align("pid", 2);
    print_align("state", 3);
    print_align("active thread\n", 4);
    while (list_hasNext(node)) {
        struct process *proc = LIST_ENTRY(node, struct process, node);
        printf("%s", proc->name);
        print_align(int_to_string(proc->pid), 2);
        print_align(p_state_to_string(proc_get_state(proc)), 3);
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

    get_default_dis_driver()->dis_setcol(0xff0000, 0x0);
    
    printf("\"GAMER GOD MOONMOON\"?\n");
    
    get_default_dis_driver()->dis_setcol(0xffffff, 0x0);

}
