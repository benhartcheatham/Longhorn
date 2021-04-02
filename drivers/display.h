/* Declares the interface of a generic display driver, as well as a function to get a default driver (currently a VESA driver)
 * and any extra utility functions that aren't directly supported by the interface. */
#ifndef _DISPLAY_H
#define _DISPLAY_H

/* includes */
#include <stdint.h>

/* defines */

/* structs */
struct display {
    /** init function for the driver 
     * 
     * @param aux: determined by implementation 
     */
    void (*dis_init)(void *aux);

    /** sets the coordinates of the cursor
     * 
     * @param x: x coordinate to set cursor to
     * @param y: y coordinate to set cursor to
     */
    void (*dis_setcur)(uint32_t x, uint32_t y);

    /** gets whether the cursor is currently visible
     * not garunteed to be implemented
     * 
     * @return 1 for if cursor is visble, 0 otherwise
     */
    uint32_t (*dis_getcur_vis)(void);

    /** gets the x coordinate of the cursor
     * 
     * @return the x coordinate of the cursor
     */
    uint32_t (*dis_getx)(void);

    /** gets the y coordinate of the cursor
     * 
     * @return the y coordinate of the cursor
     */
    uint32_t (*dis_gety)(void);

    /** returns the number of cursor columns
     * not garunteed to be implemented
     * 
     * @return number of cursor columns
     */
    uint32_t (*dis_getn_cols)(void);

    /** returns the number of cursor rows
     * not garunteed to be implemented
     * 
     * @return number of cursor rows
     */
    uint32_t (*dis_getn_rows)(void);

    /** shows the cursor on the screen */
    void (*dis_scur)(void);

    /** hides the cursor from the screen */
    void (*dis_hcur)(void);

    /** prints a character to the current cursor position
     * 
     * @param c: character to print to the screen
     */
    void (*dis_putc)(char c);

    /** prints a null-terminated string to the current cursor position
     * 
     * @param s: null-terminated string to print to the screen
     */
    void (*dis_puts)(const char *s);

    /** prints a null-terminated string to the specified cursor position
     * not garunteed to be implemented
     * 
     * @param s: null-terminated string to print to the screen
     * @param x: x coordinate to print to the screen at
     * @param y: y coordinate to print to the screen at
     */
    void (*dis_putats)(const char *s, uint32_t x, uint32_t y);

    /** prints a backspace at the current cursor position
     * not garunteed to be implemented
     */
    void (*dis_backspace)(void);

    /** prints a picture at buf at location (x,y)
     * not garunteed to be implemented
     * 
     * @param buf: the data of the image
     * @param x: the x coordinate to draw the picture to (in pixels)
     * @param y: the y coordinate to draw the picture to (in pixels) 
     */
    void (*dis_draw)(void *buf, uint32_t x, uint32_t y);

    /** clears the screen */
    void (*dis_clear)(void);

    /** sets the color to draw
     * not garunteed to be implemented
     * 
     * @param fg: the color of the foreground
     * @param bg: the color of the background
     */
    void (*dis_setcol)(uint32_t fg, uint32_t bg);
};

/* typedefs */
typedef struct display display_t;

/* functions */
display_t *get_default_dis_driver();
void display_init(void *aux);

#endif