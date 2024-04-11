#ifndef ABYSS_FONT_H
#define ABYSS_FONT_H

#include <stdint.h>

struct font_glyph {
    uint16_t code;
    uint16_t frame_index;
    uint8_t  width;
    uint8_t  height;
};

typedef struct font_s {
    struct font_glyph *glyphs;
    uint32_t           glyph_count;
} font_t;

font_t            *font_load(const char *path);
void               font_free(font_t *font);
struct font_glyph *font_get_glyph(font_t *font, uint16_t code);

#endif // ABYSS_FONT_H
