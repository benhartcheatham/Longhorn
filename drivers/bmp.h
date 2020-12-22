#ifndef BMP_H
#define BMP_H

/* includes */
#include <stdint.h>
#include <stddef.h>
#include "vesa.h"
#include "../libk/stream.h"

/* defines */
#define HEADER_SIZE sizeof(struct bmp_header)
#define HEADER_INFO_SIZE sizeof(struct bmp_info_header)
#define FILE_HEADER_SIZE sizeof(struct bmp_file_header)

/* structs */

struct bmp_info_header {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_pixels_pm;
    uint32_t y_pixels_pm;
    uint32_t colors_used;
    uint32_t important_colors;
};

struct bmp_color_table_entry {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t reserved;
};

struct bmp_header {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
};

struct __attribute__((__packed__)) bmp_file_header {
    struct bmp_header header;
    struct bmp_info_header info_header;
    struct bmp_color_table_entry *color_table;
    uint8_t *data;
};

/* typedefs */
typedef struct bmp_info_header bmp_info_header_t;
typedef struct bmp_color_table_entry bmp_color_entry_t;
typedef struct bmp_header bmp_header_t;
typedef struct bmp_file_header bmp_file_header_t;

/* functions */
int read_bmp_header(uint8_t *file, bmp_file_header_t *buf);
int read_bmp_data(bmp_file_header_t *header, std_stream *in); // this should be updated to write to a file or stream
void draw_bmp_data(bmp_file_header_t *header, uint32_t x, uint32_t y);
void print_bmp_header(bmp_file_header_t *header);
void bmp_change_color(bmp_file_header_t *header, uint32_t old, uint32_t new);
void bmp_remove_all(bmp_file_header_t *header, uint32_t color);
#endif