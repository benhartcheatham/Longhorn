#ifndef _KEYBOARD_H
#define _KEYBOARD_H

/* includes */

/* defines */
#define TERMINAL_LIMIT 256
#define KC_MAX 57
#define ENTER 0x1C
#define BACKSPACE 0x0E
#define SHIFT_PRESSED 0x2A
#define SHIFT_RELEASED 0xAA
#define CAPS_LOCK_PRESSED 0x3A
#define SPACE 0x39

/* structs */

/* typedefs */

/* functions */
void init_keyboard();

#endif