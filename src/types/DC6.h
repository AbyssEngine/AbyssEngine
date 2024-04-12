#ifndef ABYSS_DC6_H
#define ABYSS_DC6_H

#include <stdint.h>

struct DC6FrameHeader {
    uint32_t flipped;
    uint32_t width;
    uint32_t height;
    int32_t  offset_x;
    int32_t  offset_y;
    uint32_t unknown;
    uint32_t next_block;
    uint32_t data_length;
};

struct DC6Frame {
    struct DC6FrameHeader header;
    uint8_t              *frame_data;
    char                  terminator[3];
    uint8_t              *indexed_pixel_data;
};

struct DC6Header {
    int32_t  version;
    uint32_t flags;
    uint32_t encoding;
    uint8_t  termination[4];
    uint32_t directions;
    uint32_t frames_per_direction;
};

struct DC6 {
    struct DC6Header header;
    uint32_t        *frame_pointers;
    struct DC6Frame *frames;
};

struct DC6 *dc6_load(const char *path);
void        dc6_free(struct DC6 *DC6);
void        dc6_decode_frame(const struct DC6 *DC6, uint32_t frame_index);

#endif // ABYSS_DC6_H
