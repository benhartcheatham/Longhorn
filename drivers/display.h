#ifndef _DISPLAY_H
#define _DISPLAY_H

/* includes */
#include <stdint.h>

/* structs */
struct display {
    void (*dis_init)(void *aux);
    void (*dis_setcur)(uint32_t x, uint32_t y);
    uint32_t (*dis_getcur_vis)(void);    // doesn't have to be implemented
    uint32_t (*dis_getx)(void);
    uint32_t (*dis_gety)(void);
    uint32_t (*dis_getn_cols)(void);    // doesn't have to be implemented
    uint32_t (*dis_getn_rows)(void);    // doesn't have to be implemented
    void (*dis_scur)(void);
    void (*dis_hcur)(void);
    void (*dis_putc)(char c);
    void (*dis_puts)(const char *s);
    void (*dis_putats)(const char *s, uint32_t x, uint32_t y);  // doesn't have to be implemented
    void (*dis_backspace)(void);    // doesn't have to be implemented
    void (*dis_draw)(void *buf, uint32_t x, uint32_t y);    // doesn't have to be implemented
    void (*dis_clear)(void);
    void (*dis_setcol)(uint32_t fg, uint32_t bg);   // doesn't have to be implemented
};

/* typedefs */
typedef struct display display_t;

/* functions */
display_t *get_default_dis_driver();
void init_display(void *aux);

#endif