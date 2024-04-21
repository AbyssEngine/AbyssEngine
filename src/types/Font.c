#include "Font.h"

#include "../common/AbyssConfiguration.h"
#include "../common/FileManager.h"
#include "../common/Logging.h"
#include <stdlib.h>
#include <string.h>

typedef struct FontGlyph {
    uint16_t code;
    uint16_t frame_index;
    uint8_t  width;
    uint8_t  height;
} FontGlyph;

struct Font {
    FontGlyph *glyphs;
    uint32_t   glyph_count;
};

const struct FontGlyph *Font__GetGlyph(const struct Font *font, uint16_t code);

struct Font *Font_Load(const char *path) {
    char path_fixed[4096];
    memset(path_fixed, 0, 4096);

    snprintf(path_fixed, 4096, path, AbyssConfiguration_GetLocale());
    strcat(path_fixed, ".tbl");
    struct MpqStream *stream = FileManager_OpenFile(path_fixed);

    struct Font *result = malloc(sizeof(struct Font));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct Font));
    result->glyphs = calloc(0, sizeof(struct FontGlyph));

    char       magic[5];
    const char test[5] = "Woo!\x01";
    MpqStream_Read(stream, magic, 0, 5);
    if (memcmp(magic, test, 5) != 0) {
        LOG_FATAL("Failed to load Font '%s' due to invalid header.", path);
    }

    MpqStream_Seek(stream, 7, SEEK_CUR); // skip 7 unknown bytes

    while (!MpqStream_GetIsEof(stream)) {
        result->glyphs = realloc(result->glyphs, sizeof(struct FontGlyph) * (++result->glyph_count));
        FAIL_IF_NULL(result->glyphs);
        struct FontGlyph *glyph = &result->glyphs[result->glyph_count - 1];

        MpqStream_Read(stream, &glyph->code, 0, 2);
        MpqStream_Seek(stream, 1, SEEK_CUR); // skip 1 unknown byte
        MpqStream_Read(stream, &glyph->width, 0, 1);
        MpqStream_Read(stream, &glyph->height, 0, 1);
        MpqStream_Seek(stream, 3, SEEK_CUR); // skip 3 unknown bytes
        MpqStream_Read(stream, &glyph->frame_index, 0, 2);

        if (glyph->frame_index == 0xFFFF) {
            // TODO: What is this about? -1?
            glyph->frame_index = 1;
            LOG_WARN("Font '%s' has a glyph with frame index 0xFFFF for glyph %d.", path, result->glyph_count - 1);
        }

        MpqStream_Seek(stream, 4, SEEK_CUR); // skip 4 unknown bytes
    }

    MpqStream_Destroy(&stream);

    return result;
}

void Font_Destroy(struct Font *font) {
    free(font->glyphs);
    free(font);
}

void Font_GetGlyphMetrics(const Font *font, const uint16_t code, uint16_t *frame_index, uint8_t *width,
                          uint8_t *height) {
    const struct FontGlyph *glyph = Font__GetGlyph(font, code);
    if (glyph == NULL) {
        glyph = Font__GetGlyph(font, '?');
        if (glyph == NULL) {
            LOG_FATAL("Failed to get glyph metrics for code %d.", code);
        }
    }

    if (frame_index != NULL) {
        *frame_index = glyph->frame_index;
    }

    if (width != NULL) {
        *width = glyph->width;
    }

    if (height != NULL) {
        *height = glyph->height;
    }
}

const struct FontGlyph *Font__GetGlyph(const struct Font *font, const uint16_t code) {
    for (uint32_t i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].code == code) {
            return &font->glyphs[i];
        }
    }
    return NULL;
}
