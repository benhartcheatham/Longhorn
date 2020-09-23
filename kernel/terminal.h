#ifndef _TERMINAL_H
#define _TERMINAL_H

/* includes */

/* defines */
#define FULL_LOGO 2
#define HALF_LOGO 1
/* structs */

/* typedefs */
typedef void (terminal_command) (void *line);

/* functions */
void terminal_init();
void print_logo();

#endif