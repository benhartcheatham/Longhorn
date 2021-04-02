/* This file is for reading a .bmp file for an image. */
#include "bmp.h"

static uint8_t *start_pos = NULL;
static uint32_t img_size = 0;

/** reads the header of a bmp file at file into buf
 * 
 * @param file: pointer to the beginning of the .bmp file
 * @param buf: pointer to a bmp_file_header for the contents of the file
 * 
 * @return -1 if the file wasn't read correctly, otherwise the amount of bytes read
 */
int read_bmp_header(uint8_t *file, bmp_file_header_t *buf) {
    int num_read = 0;

    // read in bmp header
    buf->header.signature = *((uint16_t *) file);
    file += 2;
    buf->header.file_size = *((uint32_t *) file);
    file += 8;
    buf->header.data_offset = *((uint32_t *) file);
    file += 4;
    
    if (buf->header.file_size < HEADER_SIZE + HEADER_INFO_SIZE) {
        buf->data = NULL;
        return -1;
    }

    num_read += HEADER_SIZE;

    // read in bmp info header
    buf->info_header.size = *((uint32_t *) file);
    file += 4;
    buf->info_header.width = *((uint32_t *) file);
    file += 4;
    buf->info_header.height = *((uint32_t *) file);
    file += 4;
    buf->info_header.planes = *((uint16_t *) file);
    file += 2;
    buf->info_header.bpp = *((uint16_t *) file);
    file += 2;
    buf->info_header.compression = *((uint32_t *) file);
    file += 4;
    buf->info_header.image_size = *((uint32_t *) file);
    file += 4;
    buf->info_header.x_pixels_pm = *((uint32_t *) file);
    file += 4;
    buf->info_header.y_pixels_pm = *((uint32_t *) file);
    file += 4;
    buf->info_header.colors_used = *((uint32_t *) file);
    file += 4;
    buf->info_header.important_colors = *((uint32_t *) file);
    file += 4;

    num_read += HEADER_INFO_SIZE;

    // we don't support non 24-bit colors
    if (buf->info_header.bpp < 24) {
        buf->data = NULL;
        return -1;
    }
    
    buf->color_table = NULL;
    buf->data = file;
    num_read += 1;

    return num_read;
}

/** reads in a bmp image STD_STREAM_SIZE bytes at a time to a std_stream 
 * num_read is < STD_STREAM_SIZE when either an error occured or EOI is reached 
 * the pointer to the bits of the image is destroyed while reading and restored
 * after reading is done or an error occurs 
 * 
 * @param header: pointer to the bmp_file_header for the previously read file
 * @param in: stream to output .bmp contents to
 * 
 * @return number of bytes read into in
 */
int read_bmp_data(bmp_file_header_t *header, std_stream *in) {
    if (start_pos == NULL) {
        start_pos = header->data;
        img_size = header->info_header.width * header->info_header.height * header->info_header.bpp;
    }

    int num_read = 0;
    for (int i = 0; i < STD_STREAM_SIZE && header->data - start_pos < (int) img_size; i++) {
        put_std(in, *header->data);
        header->data++;
        num_read++;
    }

    if (num_read < STD_STREAM_SIZE) {
        header->data = start_pos;
        start_pos = NULL;
    }

    return num_read;
}

/** draws the bmp image pointed to by header at coordinate (x,y)
 *  
 * @param header: pointer to bmp_file_header for previously read in .bmp file
 * @param x: x coordinate to draw the picture starting at the top left of the picture
 * @param y: y coordinate to draw the picture starting at the top left of the picture
 * 
 */
void draw_bmp_data(bmp_file_header_t *header, uint32_t x, uint32_t y) {
    uint8_t *pixel = header->data;

    if (header->data == NULL)
        return;
    
    for (uint32_t i = header->info_header.height; i > 0; i--) {
        for (uint32_t j = 0; j < header->info_header.width; j++) {
            uint32_t col = 0;
            col = *pixel; //blue byte
            col |= *(pixel + 1) << 8; //green byte
            col |= *(pixel + 2) << 16;    //red byte
            vesa_draw(x + j, y + i, col);
            pixel += 3;
        }

        pixel += 4 - ((header->info_header.width * (header->info_header.bpp / 8)) % 4);
    }
}

/** changes color old to color new in the bmp image pointed to by header
 * 
 * @param header: pointer to the bmp_file_header of a previously read in .bmp file
 * @param old: color to replace as a 32 bit RGB value
 * @param new: color to replace old with as a 32 bit RGB value
 * 
 */
void bmp_change_color(bmp_file_header_t *header, uint32_t old, uint32_t new) {
    uint8_t *pixel = header->data;

    if (header->data == NULL)
        return;
    
    for (uint32_t i = header->info_header.height; i > 0; i--) {
        for (uint32_t j = 0; j < header->info_header.width; j++) {
            uint32_t col = 0;

            col = *pixel; //blue byte
            col |= *(pixel + 1) << 8; //green byte
            col |= *(pixel + 2) << 16;    //red byte

            if (col == old) {
                *pixel = new & 0xFF;
                *(pixel + 1) = new & (0xFF << 8);
                *(pixel + 2) = new & (0xFF << 16);
            }

            pixel += 3;
        }

        pixel += 4 - ((header->info_header.width * (header->info_header.bpp / 8)) % 4);
    }
}

/** sets all colors other than color to black in the bmp image pointed to by header
 * 
 * @param header: pointer to the bmp_file_header of a previously read in .bmp file
 * @param color: color to keep in the image as a 32 bit RGB value
 * 
 */
void bmp_remove_all(bmp_file_header_t *header, uint32_t color) {
    uint8_t *pixel = header->data;

    if (header->data == NULL)
        return;
    
    for (uint32_t i = header->info_header.height; i > 0; i--) {
        for (uint32_t j = 0; j < header->info_header.width; j++) {
            uint32_t col = 0;

            col = *pixel; //blue byte
            col |= *(pixel + 1) << 8; //green byte
            col |= *(pixel + 2) << 16;    //red byte

            if (col != color) {
                *pixel = 0x0;
                *(pixel + 1) = 0x0;
                *(pixel + 2) = 0x0;
            }

            pixel += 3;
        }

        pixel += 4 - ((header->info_header.width * (header->info_header.bpp / 8)) % 4);
    }
}