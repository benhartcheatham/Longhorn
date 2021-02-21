#ifndef _DISPLAY_H
#define _DISPLAY_H

/* includes */
#include <stdint.h>
#include "vesa.h"

/* structs */
struct display {
    void (*dis_init)(void *aux);
    void (*dis_setcur)(uint32_t x, uint32_t y);
    uint32_t (*dis_getx)(void);
    uint32_t (*dis_gety)(void);
    void (*dis_scur)(void);
    void (*dis_hcur)(void);
    void (*dis_putc)(char c);
    void (*dis_puts)(char *s);
    void (*dis_putat)(char c, uint32_t x, uint32_t y);  // doesn't have to be implemented
    void (*dis_backspace)(void);    // doesn't have to be implemented
    void (*dis_draw)(void *buf, uint32_t x, uint32_t y);    // doesn't have to be implemented
    void (*dis_clear)(void);
    void (*dis_setcol)(uint32_t fg, uint32_t bg);   // doesn't have to be implemented
};

/* typedefs */
typedef struct display display_t;

/* functions */
display_t *get_default_dis_driver();
#endif