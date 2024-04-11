#ifndef ABYSS_DC6_H
#define ABYSS_DC6_H

#include <stdint.h>

struct dc6_frame_header {
    uint32_t flipped;
    uint32_t width;
    uint32_t height;
    int32_t  offset_x;
    int32_t  offset_y;
    uint32_t unknown;
    uint32_t next_block;
    uint32_t data_length;
};

struct dc6_frame {
    struct dc6_frame_header header;
    uint8_t                *frame_data;
    char                    terminator[3];
    uint8_t                *indexed_pixel_data;
};

struct dc6_header {
    int32_t  version;
    uint32_t flags;
    uint32_t encoding;
    uint8_t  termination[4];
    uint32_t directions;
    uint32_t frames_per_direction;
};

struct dc6 {
    struct dc6_header header;
    uint32_t         *frame_pointers;
    struct dc6_frame *frames;
};

struct dc6 *dc6_load(const char *path);
void        dc6_free(struct dc6 *dc6);
void        dc6_decode_frame(const struct dc6 *dc6, uint32_t frame_index);

#endif // ABYSS_DC6_H
