#ifndef ABYSS_FONT_H
#define ABYSS_FONT_H

#include <stdint.h>

struct FontGlyph {
    uint16_t code;
    uint16_t frame_index;
    uint8_t  width;
    uint8_t  height;
};

struct Font {
    struct FontGlyph *glyphs;
    uint32_t          glyph_count;
};

struct Font      *font_load(const char *path);
void              font_free(struct Font *Font);
struct FontGlyph *font_get_glyph(struct Font *Font, uint16_t code);

#endif // ABYSS_FONT_H
