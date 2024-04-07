#ifndef ABYSS_DC6_H
#define ABYSS_DC6_H

#include <stdint.h>

typedef struct dc6_frame_header_s {
    uint32_t flipped;
    uint32_t width;
    uint32_t height;
    int32_t  offset_x;
    int32_t  offset_y;
    uint32_t unknown;
    uint32_t next_block;
    uint32_t data_length;
} dc6_frame_header_t;

typedef struct dc6_frame_s {
    dc6_frame_header_t header;
    uint8_t           *frame_data;
    char               terminator[3];
    uint8_t           *indexed_pixel_data;
} dc6_frame_t;

typedef struct dc6_header_s {
    int32_t  version;
    uint32_t flags;
    uint32_t encoding;
    uint8_t  termination[4];
    uint32_t directions;
    uint32_t frames_per_direction;
} dc6_header_t;

typedef struct dc6_s {
    dc6_header_t header;
    uint32_t    *frame_pointers;
    dc6_frame_t *frames;
} dc6_t;

dc6_t *dc6_load(const char *path);
void   dc6_free(dc6_t *dc6);
void   dc6_decode_frame(dc6_t *dc6, uint32_t frame_index);

#endif // ABYSS_DC6_H
