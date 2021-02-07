#ifndef _DISPLAY_H
#define _DISPLAY_H

/* includes */
#include <stdint.h>

/* defines */
#define PIXEL_WIDTH 24  // default size of a pixel

/* structs */
struct display_driver {
    uint32_t pixel_width;   // width of pixel
    void (*display_init) (uint32_t pixel_width, void *aux); // init for display driver
    int (*display_putc) (char c);   // put a char on the screen
    int (*display_setcur) (int x, int y);   // set the cursor position
    int (*display_getcurx) (struct display_driver *dd);
    int (*display_getcury) (struct display_driver *dd);
    int (*display_puti) (void *buf, int width, int height, int x, int y);   // put an image in buf with dimensions
                                                                            // widthxheight at (x,y)
    int (*display_setcol) (struct display_driver *dd, uint32_t foreground, uint32_t background);   // set color to write with
    uint32_t (*display_getfcol) (struct display_driver *dd); // get foreground color
    uint32_t (*display_getbcol) (struct display_driver *dd); // get bakcground color
    int (*display_scur) (void);     //show cursor
    int (*display_hcur) (void);     // hide cursor
    int (*display_puts) (char *string); // put a null-terminated string of chars to the screen
    int (*display_putb) (void);     // print a backspace to the screen
    int (*display_clear) (void);    // clear the screen, paints screen with background color
    int (*display_putcp) (char c, int x, int y);    // put a char at position (x,y)
};

/* typedefs */
typedef struct display_driver dis_driver_t;

/* functions */
void set_display_driver(dis_driver_t *dd);
dis_driver_t *get_display_driver();

#endif