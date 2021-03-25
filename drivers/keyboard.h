#ifndef _KEYBOARD_H
#define _KEYBOARD_H

/* includes */

/* defines */
#define SCAN_CODE_SET1

#ifdef SCAN_CODE_SET1
#define SC_TAB 0x0F
#define SC_LALT 0x38
#define SC_LSHIFT 0x2A
#define SC_LCTRL 0x1D
#define SC_CAPSLOCK 0x3A
#define SC_RSHIFT 0x36
#define SC_ENTER 0x1C
#define SC_BACKSPACE 0x0E
#define SC_ESCAPE 0x01
#define SC_NUMLOCK 0x45
#define SC_RELEASED 0x81
#endif

#ifdef SCAN_CODE_SET2
#define SC_TAB 0x0D
#define SC_LALT 0x11
#define SC_LSHIFT 0x12
#define SC_LCTRL 0x14
#define SC_CAPSLOCK 0x58
#define SC_RSHIFT 0x59
#define SC_ENTER 0x5A
#define SC_BACKSPACE 0x66
#define SC_ESCAPE 0x76
#define SC_NUMLOCK 0x77
#define SC_RELEASED 0xF0
#endif
/* structs */

/* typedefs */

/* functions */
void init_keyboard();

#endif