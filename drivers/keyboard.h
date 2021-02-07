#ifndef _KEYBOARD_H
#define _KEYBOARD_H

/* includes */
#include <stream.h>
#include "../kernel/isr.h"

/* defines */
#define TERMINAL_LIMIT 256
#define KC_MAX 57
#define ENTER 0x1C
#define BACKSPACE_KC 0x0E
#define BACKSPACE 8
#define SHIFT_PRESSED 0x2A
#define SHIFT_RELEASED 0xAA
#define CAPS_LOCK_PRESSED 0x3A
#define SPACE 0x39

/* structs */
enum keyboard_modes {COOKED = 0, RAW = 1};

struct keyboard_driver {
    enum keyboard_modes mode;
    char *buff;

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