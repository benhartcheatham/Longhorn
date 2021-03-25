#ifndef _SHELL_H
#define _SHELL_H

/* includes */
#include <stdint.h>

/* defines */
#define FULL_LOGO 2
#define HALF_LOGO 1
/* structs */

/* typedefs */
typedef void (*shell_command) (char **line, uint32_t argc);

/* functions */
void shell_init();
void print_logo();

#endif