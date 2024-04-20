#include "DC6Frame.h"
#include "../common/Logging.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct DC6FrameHeader {
    uint32_t flipped;
    uint32_t width;
    uint32_t height;
    int32_t  offset_x;
    int32_t  offset_y;
    uint32_t unknown;
    uint32_t next_block;
    uint32_t data_length;
} DC6FrameHeader;

struct DC6Frame {
    DC6FrameHeader header;
    uint8_t       *frame_data;
    char           terminator[3];
    uint8_t       *indexed_pixel_data;
};

void DC6Frame__Decode(DC6Frame *dc6_frame);

DC6Frame *DC6Frame_Create(MpqStream *mpq_stream) {
    DC6Frame *result = malloc(sizeof(DC6Frame));
    memset(result, 0, sizeof(DC6Frame));
    FAIL_IF_NULL(result);

    MpqStream_Read(mpq_stream, &result->header, 0, sizeof(DC6FrameHeader));
    result->frame_data = malloc(result->header.data_length);

    FAIL_IF_NULL(result->frame_data);

    MpqStream_Read(mpq_stream, result->frame_data, 0, result->header.data_length);
    MpqStream_Read(mpq_stream, result->terminator, 0, 3);

    DC6Frame__Decode(result);

    return result;
}

void DC6Frame_Destroy(DC6Frame **dc6_frame) {
    free((*dc6_frame)->indexed_pixel_data);
    free((*dc6_frame)->frame_data);
    free(*dc6_frame);
    *dc6_frame = NULL;
}

void DC6Frame_GetFrameSize(const DC6Frame *dc6_frame, uint32_t *width, uint32_t *height) {
    assert((width != NULL) || (height != NULL));

    if (width != NULL) {
        *width = dc6_frame->header.width;
    }

    if (height != NULL) {
        *height = dc6_frame->header.height;
    }
}

const uint8_t *DC6Frame_GetPixelData(const DC6Frame *dc6_frame) { return dc6_frame->indexed_pixel_data; }

void DC6Frame_GetFrameOffset(const DC6Frame *dc6_frame, int32_t *offset_x, int32_t *offset_y) {
    if (offset_x != NULL) {
        *offset_x = dc6_frame->header.offset_x;
    }

    if (offset_y != NULL) {
        *offset_y = dc6_frame->header.offset_y;
    }
}

void DC6Frame__Decode(DC6Frame *dc6_frame) {
    assert(dc6_frame != NULL);
    assert(dc6_frame->frame_data != NULL);

    uint32_t frame_width;
    uint32_t frame_height;
    DC6Frame_GetFrameSize(dc6_frame, &frame_width, &frame_height);
    const uint32_t total_pixels = frame_height * frame_width;

    dc6_frame->indexed_pixel_data = malloc((size_t)total_pixels);
    FAIL_IF_NULL(dc6_frame->indexed_pixel_data);
    memset(dc6_frame->indexed_pixel_data, 0, (size_t)total_pixels);

    uint32_t x      = 0;
    uint32_t y      = frame_height - 1;
    int      offset = 0;

    for (;;) {
        const int b = dc6_frame->frame_data[offset++];

        if (b == 0x80) {
            if (y == 0) {
                return;
            }
            --y;
            x = 0;
            continue;
        }

        if ((b & 0x80) > 0) {
            x += b & 0x7F;
            continue;
        }

        for (int i = 0; i < b; i++) {
            dc6_frame->indexed_pixel_data[x + (y * frame_width) + i] = dc6_frame->frame_data[offset++];
        }

        x += b;
    }
}
