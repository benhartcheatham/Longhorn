#ifndef _TERMINAL_H
#define _TERMINAL_H

/* includes */

/* defines */
#define NUM_COMMANDS 4

/* structs */

/* typedefs */
typedef void (*terminal_command) (void *aux);

/* functions */
void terminal_init();

#endif