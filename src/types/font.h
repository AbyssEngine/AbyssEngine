#ifndef ABYSS_FONT_H
#define ABYSS_FONT_H

#include <stdint.h>

typedef struct font_glyph_s {
    uint16_t code;
    uint16_t frame_index;
    uint8_t  width;
    uint8_t  height;
} font_glyph_t;

typedef struct font_s {
    font_glyph_t *glyphs;
    uint32_t      glyph_count;
} font_t;

font_t       *font_load(const char *path);
void          font_free(font_t *font);
font_glyph_t *font_get_glyph(font_t *font, uint16_t code);

#endif // ABYSS_FONT_H
