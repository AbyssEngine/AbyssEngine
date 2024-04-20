#ifndef ABYSS_DC6_H
#define ABYSS_DC6_H

#include <stdint.h>

typedef struct DC6 DC6;

DC6           *DC6_Load(const char *path);
void           DC6_Destroy(DC6 **dc6);
uint32_t       DC6_GetTotalFrameCount(const DC6 *dc6);
void           DC6_GetFrameSize(const DC6 *dc6, uint32_t frame_index, uint32_t *width, uint32_t *height);
void           DC6_GetFrameOffset(const DC6 *dc6, uint32_t frame_index, int32_t *offset_x, int32_t *offset_y);
const uint8_t *DC6_GetFramePixelData(const DC6 *dc6, uint32_t frame_index);

#endif // ABYSS_DC6_H
