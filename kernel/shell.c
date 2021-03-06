/* The default shell for the kernel. */

/*includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <mem.h>
#include "../drivers/terminal.h"
#include "../drivers/bmp.h"
#include "shell.h"
#include "thread.h"
#include "port_io.h"
#include "proc.h"

/* defines */
#define GRAPHICS_MODE 0
#define TEXT_MODE 1

#define MAX_NUM_ARGS 26
#define NUM_COMMANDS 8
#define NUM_HELP_COMMANDS (NUM_COMMANDS - 2)

#define LOGO_COLOR 0xBD5615
#define MIN_ARG_MEM 16  //small strings really screw up arg-making

/* globals */

/* shell info */
size_t last_index = 0;
char *help_commands[NUM_HELP_COMMANDS] = {"help", "shutdown", "exit", "ps", "clear", "getbuf"};
char *commands[NUM_COMMANDS] = {"help", "shutdown", "exit", "ps", "clear", "getbuf", "grub", "moon"};
struct process *shell;

/* key buffer info */
static char key_buffer[LINE_BUFFER_SIZE];
static bool cursor_on = false;

/* prototypes */
static void shell_waiter(void *aux);
static void read_stdin(struct process *active);
static uint32_t ps_get_alignment(display_t *dis, uint32_t *alignment);

/* command functions */
static void help(void *aux);
static void shutdown(void *aux);
static void ps(void *aux);
static void getbuf(void *aux);
static void grub(void *aux);
static void moon(void *aux);
static void clear(void *aux);
proc_function *command_functions[NUM_COMMANDS] = {help, shutdown, shutdown, ps, clear, getbuf, grub, moon}; // this has to be here sadly, can't be moved before the protoyypes

/* functions */

/** initializes a shell process */
void shell_init() {
    shell = proc_create("shell", shell_waiter, NULL);
    proc_set_active(shell->pid);
    shell->stdout = shell->stdin;

    line_init(get_default_line_disc(), NULL, GET_STDOUT(shell), GET_STDIN(shell), COOKED);
}

/** function for the shell process to use, constantly scans input
 * 
 * @param aux: unused
 */
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

        read_stdin(shell);

    }

}

/** reads the active process' stdin stream for input from the user
 * input is executed as a command, if available, when the ENTER key is pressed
 * 
 * @param active: pointer to active process
 */ 
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

                    void *aux[2] = {(void *) args, (void *) argc};
                    struct process *comm = proc_create("ps", command_functions[i], aux);
                    if (comm) {
                        proc_wait(comm);
                    }
                    
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
            kprintf("> ");
        }
        

        c = get_std(stdin);
    }
}

/** prints a list of available commands
 * 
 * @param line: unused
 * @param argc: unused
 */
static void help(void *aux __attribute__ ((unused))) {
    kprintf("Available Commands:\n");
    int i;
    for (i = 0; i < NUM_HELP_COMMANDS; i++) {
        kprintf("\t%s\n", help_commands[i]);
    }
}
/** shuts down the machine gracefully (only works for qemu) 
 * 
 * @param line: unused
 * @param argc: unused
 */
static void shutdown(void *aux __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}

/** prints the given command line args
 * 
 * @param line: pointer to command line args
 * @param argc: number of command line args
 */
static void getbuf(void *aux) {
    void **aux_arr = (void **) aux;
    char **args = ((char **) aux_arr[0]);
    uint32_t argc = (uint32_t) (aux_arr[1]);

    kprintf("buffer: ");
    for (uint32_t i = 0; i < argc; i++)
        kprintf("%s ", args[i]);
    kprintf("\n");
}

/** converts a thread state to a human-readable string
 *
 * @param s: state to convert
 * 
 * @return state s as a string
 */
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


/** prints info about the processes in the system
 * 
 * @param line: unused
 * @param argc: unused
 */
static void ps(void *aux __attribute__ ((unused))) {
    display_t *dis = get_default_dis_driver();
    const list_node_t *node = proc_peek_all_list();
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
        kprintf("\n");

        node = list_get_next(node);
    }

}

/** helper function for ps that gets the y coordinate of the next info string
 * 
 * @param dis: display driver used by the shell
 * @param a: alignment used in calculation
 * 
 * @return y coordinate of next info string
 */
static uint32_t ps_get_alignment(display_t *dis, uint32_t *a) {
    uint32_t next_y = (*a) * (dis->dis_getn_cols() / 8);
    *a = *a + 1;    // done this way so compiler doesn't complain
    return next_y;
}

static void clear(void *aux __attribute__ ((unused))) {
    get_default_dis_driver()->dis_clear();
}

/* novelty command */
static void grub(void *aux __attribute__ ((unused))) {
    kprintf("GRUB is ok\n\n\n\ni guess...\n");
}

/* novelty command */
static void moon(void *aux __attribute__ ((unused))) {
    kprintf("did you mean: ");

    get_default_dis_driver()->dis_setcol(0xff0000, 0x0);
    
    kprintf("\"GAMER GOD MOONMOON\"?\n");
    
    get_default_dis_driver()->dis_setcol(0xffffff, 0x0);

}
