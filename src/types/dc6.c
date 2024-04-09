#include "dc6.h"
#include "../common/fileman.h"
#include "../common/log.h"

#include <stdlib.h>
#include <string.h>

dc6_t *dc6_load(const char *path) {
    dc6_t *result = malloc(sizeof(dc6_t));
    FAIL_IF_NULL(result);

    mpq_stream_t *stream = fileman_load(path);
    mpq_stream_read(stream, &result->header, 0, sizeof(dc6_header_t));

    const uint32_t total_frames = result->header.directions * result->header.frames_per_direction;
    result->frame_pointers      = malloc(sizeof(uint32_t) * total_frames);

    FAIL_IF_NULL(result->frame_pointers);

    mpq_stream_read(stream, result->frame_pointers, 0, sizeof(uint32_t) * total_frames);

    result->frames = malloc(sizeof(dc6_frame_t) * total_frames);
    FAIL_IF_NULL(result->frames);

    for (uint32_t i = 0; i < total_frames; i++) {
        dc6_frame_t *frame = &result->frames[i];
        mpq_stream_read(stream, &frame->header, 0, sizeof(dc6_frame_header_t));
        frame->frame_data = malloc(frame->header.data_length);
        FAIL_IF_NULL(frame->frame_data);
        mpq_stream_read(stream, frame->frame_data, 0, frame->header.data_length);
        mpq_stream_read(stream, frame->terminator, 0, 3);
        dc6_decode_frame(result, i);
    }

    mpq_stream_free(stream);

    return result;
}

void dc6_decode_frame(const dc6_t *dc6, uint32_t frame_index) {
    dc6_frame_t *frame = &dc6->frames[frame_index];

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

void dc6_free(dc6_t *dc6) {
    const uint32_t total_frames = dc6->header.directions * dc6->header.frames_per_direction;
    for (int i = 0; i < total_frames; i++) {
        free(dc6->frames[i].frame_data);
        free(dc6->frames[i].indexed_pixel_data);
    }
    free(dc6->frames);
    free(dc6->frame_pointers);
    free(dc6);
}
