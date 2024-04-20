#ifndef ABYSS_DC6FRAME_H
#define ABYSS_DC6FRAME_H

#include "../common/MpqStream.h"
#include <stdint.h>

typedef struct DC6Frame DC6Frame;

DC6Frame      *DC6Frame_Create(MpqStream *mpq_stream);
void           DC6Frame_Destroy(DC6Frame **dc6_frame);
void           DC6Frame_GetFrameSize(const DC6Frame *dc6_frame, uint32_t *width, uint32_t *height);
void           DC6Frame_GetFrameOffset(const DC6Frame *dc6_frame, int32_t *offset_x, int32_t *offset_y);
const uint8_t *DC6Frame_GetPixelData(const DC6Frame *dc6_frame);

#endif // ABYSS_DC6FRAME_H
