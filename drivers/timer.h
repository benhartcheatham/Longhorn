/* Defines functionality for timer interrupts. */
#ifndef _TIMER_H
#define _TIMER_H

/* includes */
#include <stdint.h>

/* defines */
#define R_FREQ 100  //the recommended frequency to run the system on (completely made up as of now)

/* structs */

/* typedefs */

/* functions */
void init_timer(uint32_t freq);

#endif