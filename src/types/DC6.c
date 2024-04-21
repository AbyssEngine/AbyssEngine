#include "DC6.h"
#include "../common/FileManager.h"
#include "../common/Logging.h"
#include "DC6Frame.h"

#include <assert.h>
#include <stdlib.h>

typedef struct DC6Header {
    int32_t  version;
    uint32_t flags;
    uint32_t encoding;
    uint8_t  termination[4];
    uint32_t directions;
    uint32_t frames_per_direction;
} DC6Header;

struct DC6 {
    struct DC6Header header;
    uint32_t        *frame_pointers;
    DC6Frame       **dc6_frames;
};

DC6 *DC6_Load(const char *path) {
    DC6 *result = malloc(sizeof(DC6));
    FAIL_IF_NULL(result);

    MpqStream *stream = FileManager_OpenFile(path);
    MpqStream_Read(stream, &result->header, 0, sizeof(DC6Header));

    const uint32_t total_frames = result->header.directions * result->header.frames_per_direction;
    result->frame_pointers      = malloc(sizeof(uint32_t) * total_frames);

    FAIL_IF_NULL(result->frame_pointers);

    MpqStream_Read(stream, result->frame_pointers, 0, sizeof(uint32_t) * total_frames);

    result->dc6_frames = malloc(sizeof(DC6Frame *) * total_frames);
    FAIL_IF_NULL(result->dc6_frames);

    for (uint32_t i = 0; i < total_frames; i++) {
        result->dc6_frames[i] = DC6Frame_Create(stream);
    }

    MpqStream_Destroy(&stream);

    return result;
}

void DC6_Destroy(struct DC6 **dc6) {
    assert(dc6 != NULL);

    const uint32_t total_frames = DC6_GetTotalFrameCount(*dc6);
    for (uint32_t i = 0; i < total_frames; i++) {
        DC6Frame_Destroy(&(*dc6)->dc6_frames[i]);
    }

    free((*dc6)->dc6_frames);
    free((*dc6)->frame_pointers);

    free(*dc6);
    *dc6 = NULL;
}

uint32_t DC6_GetTotalFrameCount(const DC6 *dc6) { return dc6->header.directions * dc6->header.frames_per_direction; }

void DC6_GetFrameSize(const DC6 *dc6, const uint32_t frame_index, uint32_t *width, uint32_t *height) {
    assert(dc6 != NULL);
    assert(frame_index < (dc6->header.directions * dc6->header.frames_per_direction));

    DC6Frame_GetFrameSize(dc6->dc6_frames[frame_index], width, height);
}

const uint8_t *DC6_GetFramePixelData(const DC6 *dc6, const uint32_t frame_index) {
    assert(dc6 != NULL);
    assert(frame_index < (dc6->header.directions * dc6->header.frames_per_direction));

    return DC6Frame_GetPixelData(dc6->dc6_frames[frame_index]);
}

void DC6_GetFrameOffset(const DC6 *dc6, const uint32_t frame_index, int32_t *offset_x, int32_t *offset_y) {
    assert(dc6 != NULL);
    assert(frame_index < (dc6->header.directions * dc6->header.frames_per_direction));

    DC6Frame_GetFrameOffset(dc6->dc6_frames[frame_index], offset_x, offset_y);
}
