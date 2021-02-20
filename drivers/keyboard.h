#ifndef _KEYBOARD_H
#define _KEYBOARD_H

/* includes */
#include <stdbool.h>
#include <stream.h>
#include "../kernel/isr.h"

/* defines */
#define TERMINAL_LIMIT STD_STREAM_SIZE

// ascii defines
#define ASCII_BACKSPACE 8
#define ASCII_TAB 9
#define ASCII_LCTRL 17 // this one may be wrong
#define ASCII_ESCAPE 27
#define ASCII_SHIFT_PRESSED 15

// keycode defines
#define KC_ESCAPE 0x1
#define KC_BACKSPACE 0x0E
#define KC_TAB 0x0F
#define KC_ENTER 0x1C
#define KC_LCTRL 0x1D
#define KC_LSHIFT 0x2A
#define KC_RSHIFT 0x36
#define KC_LEFT_ALT 0x38
#define KC_SPACE 0x39
#define KC_CAPS_LOCK 0x3A
#define KC_LSHIFT_RELEASED 0xAA
#define KC_RSHIFT_RELEASED 0xB6

/* structs */
enum keyboard_modes {KCOOKED = 0, KRAW = 1};

struct keyboard_driver {
    enum keyboard_modes mode;
    char *buff;

    bool capitalize;    // whether the next char should be captialized
    bool ctrl_pressed;  // whether the control key is pressed
    bool alt_pressed;   // whether the alt key is pressed

    void (*keyboard_init) (void *aux);
    int (*keyboard_mode) (enum keyboard_modes mode);
    int (*keyboard_handler) (struct register_frame *r);
};


/* typedefs */
typedef struct keyboard_driver key_driver_t;
typedef enum keyboard_modes key_modes_t;

/* functions */
void init_keyboard(void *aux);
int keyboard_set_mode(key_modes_t mode);
int keyboard_handler(struct register_frame *r);
int keyboard_direct (std_stream *in);



#endif