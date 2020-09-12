#include "terminal.h"
#include "proc.h"
#include "port_io.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"

size_t last_index = 0;
char *commands[NUM_COMMANDS] = {"shutdown", "ps", "grub", "moon"};
uint32_t terminal_pid;

static char key_buffer[TERMINAL_LIMIT + 1];
static uint32_t key_buf_i = 0;

/* static functions */
static void terminal_waiter(void *aux);
static void read_stdin();
static void append_to_buffer(char c);
static void shrink_buffer();
static void flush_buffer();

/* command functions */
static void shutdown(void *aux);
static void ps(void *aux);
static void grub(void *aux);
static void moon(void *aux);

terminal_command command_functions[NUM_COMMANDS] = {shutdown, ps, grub, moon};

void terminal_init() {
    terminal_pid = proc_create("terminal", terminal_waiter, NULL);
    proc_set_active(terminal_pid);
    flush_buffer();

}

static void terminal_waiter(void *aux __attribute__ ((unused))) {

    //the while loop breaks the keyboard entirely for some reason
    while (1) {
        read_stdin();
    }

}

static void read_stdin() {
    struct process *active = proc_get_active();
    std_stream *stdin = &active->stdin;

    char c = get_std(stdin);
    while (c != -1) {
        if (c == '\n') {
            vga_print_char('\n');

            int i;
            for (i = 0; i < NUM_COMMANDS; i++)
                if (strcmp(key_buffer, commands[i]) == 0)
                    command_functions[i](NULL);
            
            flush_buffer();

        } else if (c == '\b') {
            if (key_buf_i > 0) {
                //get rid of character the backspace is upposed to get rid of
                get_std(stdin);
                shrink_buffer(1);
                print_backspace();
            }
        } else {
            append_to_buffer(c);
            vga_print_char(c);
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

/* shutsdown the machine gracefully (only works for qemu) */
static void shutdown(void *aux __attribute__ ((unused))) {
    outw(0x604, 0x2000);
}

static void ps(void *aux __attribute__ ((unused))) {
    //shouldn't be using the list directly, should be using an iterator with const nodes
    list_node_t *node = proc_get_all_list()->head.next;
    
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

        node = node->next;
    }
}

static void grub(void *aux __attribute__ ((unused))) {
    printf("GRUB is ok\n\n\n\ni guess...\n");
}

static void moon(void *aux __attribute__ ((unused))) {
    printf("did you mean: ");
    set_fg_color(VGA_COLOR_RED);
    printf("\"GAMER GOD MOONMOON\"?\n");
    set_default_color();
}
