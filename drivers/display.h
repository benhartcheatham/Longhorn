#ifndef _DISPLAY_H
#define _DISPLAY_H

/* includes */
#include <stdint.h>

/* defines */
#define PIXEL_WIDTH 24  // default size of a pixel

/* structs */
struct display_driver {
    uint32_t pixel_width;   // width of pixel
    int (*init) (void *aux); // init for display driver
    int (*putc) (char c);   // put a char on the screen
    int (*setcur) (int x, int y);   // set the cursor position
    int (*getcurx) (void);
    int (*getcury) (void);
    int (*puti) (void *buf, int width, int height, int x, int y);   // put an image in buf with dimensions
                                                                    // widthxheight at (x,y)
    int (*setcol) (uint32_t foreground, uint32_t background);   // set color to write with
    uint32_t (*getfcol) (void); // get foreground color
    uint32_t (*getbcol) (void); // get bakcground color
    int (*scur) (void);     //show cursor
    int (*hcur) (void);     // hide cursor
    int (*puts) (char *string); // put a null-terminated string of chars to the screen
    int (*backspace) (void);     // print a backspace to the screen
    int (*clear) (void);    // clear the screen, paints screen with background color
    int (*putcp) (char c, int x, int y);    // put a char at position (x,y)
};

/* typedefs */
typedef struct display_driver dis_driver_t;

/* functions */
int display_init(void *aux);
void set_display_driver(dis_driver_t *dd);
dis_driver_t *get_default_dd();

#endif