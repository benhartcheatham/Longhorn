#include "bmp.h"

static uint8_t *start_pos = NULL;
static uint32_t img_size = 0;

int read_bmp_header(uint8_t *file, bmp_file_header_t *buf) {
    int num_read = 0;

    // read in bmp header
    buf->header.signature = *((uint16_t *) file);
    file += 2;
    buf->header.file_size = *((uint32_t *) file);
    file += 8;
    buf->header.data_offset = *((uint32_t *) file);
    file += 4;
    
    if (buf->header.file_size < HEADER_SIZE + HEADER_INFO_SIZE)
        return -1;
    
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
    if (buf->info_header.bpp < 24)
        return -1;
    
    buf->color_table = NULL;
    buf->data = file;
    num_read += 1;

    return num_read;
}

/* reads in a bmp image STD_STREAM_SIZE bytes at a time to a std_stream 
   num_read is < STD_STREAM_SIZE when either an error occured or EOI is reached 
   the pointer to the bits of the image is destroyed while reading and restored
   after reading is done or an error occurs */
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

void draw_bmp_data(bmp_file_header_t *header, uint32_t x, uint32_t y) {
    uint8_t *pixel = header->data;

    for (uint32_t i = 0; i < header->info_header.height; i++)
        for (uint32_t j = 0; j < header->info_header.width; j++) {
            uint32_t col = *pixel << 16; //red byte
            col |= *(pixel + 1) << 8; //green byte
            col |= *(pixel + 2);    //blue byte
            vesa_draw(x + j, y + i, col);
            pixel += 3;
        }
}