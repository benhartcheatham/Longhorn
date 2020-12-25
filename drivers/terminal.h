#ifndef _TERMINAL_H
#define _TERMINAL_H

/* includes */
#include <stdint.h>
#include "keyboard.h"
#include "../kernel/proc.h"

/* defines */

/* structs */
enum TERMINAL_MODE {
    RAW,
    FILTERED,
};

enum TERMINAL_DIS_MODE {
    D_RAW,
    D_FILTERED,
    D_CHAR_ONLY,
    NONE,
};

enum TERMINAL_DRIVER_MODE {
    VGA,
    VESA,
    DR_NONE,
};

struct terminal {
    uint32_t term_id;

    struct process *out;

    enum TERMINAL_MODE mode;
    enum TERMINAL_DIS_MODE dmode;
    enum TERMINAL_DRIVER_MODE drmode;

};

/* typedefs */
typedef enum TERMINAL_MODE tmode_t;
typedef enum TERMINAL_DIS_MODE tdmode_t;
typedef enum TERMINAL_DRIVER_MODE tdrmode_t;

/* functions */

/* setter functions */
void terminal_init(struct terminal *t);
int terminal_out(struct terminal *t, struct process *out);
int terminal_mode(struct terminal *t, tmode_t mode);
int terminal_dmode(struct terminal *t, tdmode_t mode);
int terminal_drmode(struct terminal *t, tdrmode_t mode);
int terminal_active(struct terminal *t);

/* driver functions */
int terminal_put(uint8_t c);
int terminal_scur();
int terminal_hcur();
int terminal_p(char *str);
int terminal_pln(char *str);
int terminal_pback();
int terminal_fgc(uint32_t col);
int terminal_bgc(uint32_t col);

#endif