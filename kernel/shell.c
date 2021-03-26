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

#define MAX_NUM_ARGS 26
#define NUM_COMMANDS 7
#define NUM_HELP_COMMANDS (NUM_COMMANDS - 2)

#define LOGO_COLOR 0xBD5615
#define MIN_ARG_MEM 16  //small strings really screw up arg-making

/* shell info */
size_t last_index = 0;
char *help_commands[NUM_HELP_COMMANDS] = {"help", "shutdown", "exit", "ps", "getbuf"};
char *commands[NUM_COMMANDS] = {"help", "shutdown", "exit", "ps", "grub", "moon", "getbuf"};
uint32_t shell_pid;

/* logo variables */
extern char _binary_assets_longhorn_logo_bmp_start;
uint8_t *header_addr = (uint8_t *) &_binary_assets_longhorn_logo_bmp_start;
bmp_file_header_t header;

/* key buffer info */
static char key_buffer[LINE_BUFFER_SIZE];
static bool cursor_on = false;

/* static functions */
static void shell_waiter(void *aux);
static void read_stdin(struct process *active);
static uint32_t ps_get_alignment(display_t *dis, uint32_t *alignment);

/* command functions */
static void help(char **line, uint32_t argc);
static void shutdown(char **line, uint32_t argc);
static void ps(char **line, uint32_t argc);
static void getbuf(char **line, uint32_t argc);
static void grub(char **line, uint32_t argc);
static void moon(char **line, uint32_t argc);
shell_command command_functions[NUM_COMMANDS] = {help, shutdown, shutdown, ps, grub, moon, getbuf};

/* initializes a shell process */
void shell_init() {
    shell_pid = proc_create("shell", shell_waiter, NULL);
    proc_set_active(shell_pid);

    line_init(get_default_line_disc(), GET_STDIN(proc_get_active()), COOKED);
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
    while (c != -1) {
        if (c == '\n') {
            dis->dis_hcur();
            ld->line_outbufn(ld, key_buffer, LINE_BUFFER_SIZE);
            key_buffer[strlen(key_buffer) - 1] = 0; // remove the newline

            char *args[MAX_NUM_ARGS];
            uint32_t argc = 0;
            char *token = strtok(key_buffer, " ");
            size_t arg_length = strlen(token);

            // kmalloc is apparently still really buggy with small allocations
            if (arg_length < MIN_ARG_MEM)
                arg_length = MIN_ARG_MEM;

            args[0] = kmalloc(arg_length);
            strcpy(args[0], token);
            argc++;

            while ((token = strtok(NULL, " ")) != NULL && argc < MAX_NUM_ARGS) {
                arg_length = strlen(token);
                
                if (arg_length < MIN_ARG_MEM)
                    arg_length = MIN_ARG_MEM;

                args[argc] = kmalloc(arg_length);
                strcpy(args[argc], token);
                argc++;
            }

            int i;
            for (i = 0; i < NUM_COMMANDS; i++) {

                if (strcmp(trim(args[0]), commands[i]) == 0) {
                    /* this should make a new process/thread, but I need to be able
                    *  to wait on child processes  */

                    command_functions[i](args, argc);
                    break;
                }
            }
            flush_std(stdin);
            ld->line_flush(ld);

            for (uint32_t i = 0; i < argc; i++) {
                memset(args[i], 0, strlen(args[i]));
                kfree(args[i]);
            }

            memset(args, 0, MAX_NUM_ARGS * sizeof(char *));
            memset(key_buffer, 0, LINE_BUFFER_SIZE);
            printf("> ");
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
static void help(char **line __attribute__ ((unused)), uint32_t argc __attribute__ ((unused))) {
    printf("Available Commands:\n");
    int i;
    for (i = 0; i < NUM_HELP_COMMANDS; i++) {
        printf("\t%s\n", help_commands[i]);
    }
}
/* shuts down the machine gracefully (only works for qemu) */
static void shutdown(char **line __attribute__ ((unused)), uint32_t argc __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}

static void getbuf(char **line, uint32_t argc) {
    printf("buffer: ");
    for (uint32_t i = 0; i < argc; i++)
        printf("%s ", line[i]);
    printf("\n");
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


/* TODO - update this function to use print_at/set_cursor calls */
static void ps(char **line __attribute__ ((unused)), uint32_t argc __attribute__ ((unused))) {
    display_t *dis = get_default_dis_driver();
    const list_node *node = proc_peek_all_list();
    uint32_t alignment = 0;

    char *headers[4] = {"name", "pid", "state", "active thread\n"};
    for (int i = 0; i < 4; i++)
        dis->dis_putats(headers[i], ps_get_alignment(dis, &alignment), dis->dis_gety());
    
    while (list_hasNext(node)) {
        alignment = 0;
        struct process *proc = LIST_ENTRY(node, struct process, node);
        dis->dis_putats(proc->name, ps_get_alignment(dis, &alignment), dis->dis_gety());
        dis->dis_putats(int_to_string(proc->pid), ps_get_alignment(dis, &alignment), dis->dis_gety());
        dis->dis_putats(p_state_to_string(proc_get_state(proc)), ps_get_alignment(dis, &alignment), dis->dis_gety());
        dis->dis_putats(proc->active_thread->name, ps_get_alignment(dis, &alignment), dis->dis_gety());
        printf("\n");

        node = list_get_next(node);
    }

}

static uint32_t ps_get_alignment(display_t *dis, uint32_t *a) {
    uint32_t next_y = (*a) * (dis->dis_getn_cols() / 8);
    *a = *a + 1;    // done this way so compiler doesn't complain
    return next_y;
}

/* novelty command */
static void grub(char **line __attribute__ ((unused)), uint32_t argc __attribute__ ((unused))) {
    printf("GRUB is ok\n\n\n\ni guess...\n");
}

/* novelty command */
static void moon(char **line __attribute__ ((unused)), uint32_t argc __attribute__ ((unused))) {
    printf("did you mean: ");

    get_default_dis_driver()->dis_setcol(0xff0000, 0x0);
    
    printf("\"GAMER GOD MOONMOON\"?\n");
    
    get_default_dis_driver()->dis_setcol(0xffffff, 0x0);

}
