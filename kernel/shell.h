#ifndef _SHELL_H
#define _SHELL_H

/* includes */

/* defines */
#define FULL_LOGO 2
#define HALF_LOGO 1
/* structs */

/* typedefs */
typedef void (shell_command) (void *line);

/* functions */
void shell_init();
void print_logo();

#endif