#include "DC6.h"
#include "../common/FileManager.h"
#include "../common/Logging.h"

#include <stdlib.h>
#include <string.h>

struct DC6 *dc6_load(const char *path) {
    struct DC6 *result = malloc(sizeof(struct DC6));
    FAIL_IF_NULL(result);

    struct MpqStream *stream = FileManager_OpenFile(path);
    MpqStream_Read(stream, &result->header, 0, sizeof(struct DC6Header));

    const uint32_t total_frames = result->header.directions * result->header.frames_per_direction;
    result->frame_pointers      = malloc(sizeof(uint32_t) * total_frames);

    FAIL_IF_NULL(result->frame_pointers);

    MpqStream_Read(stream, result->frame_pointers, 0, sizeof(uint32_t) * total_frames);

    result->frames = malloc(sizeof(struct DC6Frame) * total_frames);
    FAIL_IF_NULL(result->frames);

    for (uint32_t i = 0; i < total_frames; i++) {
        struct DC6Frame *frame = &result->frames[i];
        MpqStream_Read(stream, &frame->header, 0, sizeof(struct DC6FrameHeader));
        frame->frame_data = malloc(frame->header.data_length);
        FAIL_IF_NULL(frame->frame_data);
        MpqStream_Read(stream, frame->frame_data, 0, frame->header.data_length);
        MpqStream_Read(stream, frame->terminator, 0, 3);
        dc6_decode_frame(result, i);
    }

    MpqStream_Destroy(&stream);

    return result;
}

void dc6_decode_frame(const struct DC6 *DC6, uint32_t frame_index) {
    struct DC6Frame *frame = &DC6->frames[frame_index];

    frame->indexed_pixel_data = malloc((size_t)frame->header.width * frame->header.height);
    FAIL_IF_NULL(frame->indexed_pixel_data);
    memset(frame->indexed_pixel_data, 0, (size_t)frame->header.width * frame->header.height);

    uint32_t x      = 0;
    uint32_t y      = frame->header.height - 1;
    int      offset = 0;

    for (;;) {
        const int b = frame->frame_data[offset++];

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
            frame->indexed_pixel_data[x + (y * frame->header.width) + i] = frame->frame_data[offset++];
        }

        x += b;
    }
}

void dc6_free(struct DC6 *DC6) {
    const uint32_t total_frames = DC6->header.directions * DC6->header.frames_per_direction;
    for (uint32_t i = 0; i < total_frames; i++) {
        free(DC6->frames[i].frame_data);
        free(DC6->frames[i].indexed_pixel_data);
    }
    free(DC6->frames);
    free(DC6->frame_pointers);
    free(DC6);
}
