#ifndef ABYSS_FONT_H
#define ABYSS_FONT_H

#include <stdint.h>

typedef struct Font Font;

Font *Font_Load(const char *path);
void  Font_Destroy(Font *font);
void  Font_GetGlyphMetrics(const Font *font, uint16_t code, uint16_t *frame_index, uint8_t *width, uint8_t *height);

#endif // ABYSS_FONT_H
