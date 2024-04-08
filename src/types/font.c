#include "font.h"

#include "../common/config.h"
#include "../common/fileman.h"
#include "../common/log.h"
#include "../common/mpq_stream.h"
#include <stdlib.h>
#include <string.h>

font_t *font_load(const char *path) {
    char *path_fixed = malloc(4096);
    memset(path_fixed, 0, 4096);
    snprintf(path_fixed, 4096, path, config->locale);
    strcat(path_fixed, ".tbl");
    mpq_stream_t *stream = fileman_load(path_fixed);

    font_t *result = malloc(sizeof(font_t));
    memset(result, 0, sizeof(font_t));
    result->glyphs = calloc(0, sizeof(font_glyph_t));

    char magic[5];
    char test[5] = "Woo!\x01";
    mpq_stream_read(stream, magic, 0, 5);
    if (memcmp(magic, test, 5) != 0) {
        LOG_FATAL("Failed to load font '%s' due to invalid header.", path);
    }

    mpq_stream_seek(stream, 7, SEEK_CUR); // skip 7 unknown bytes

    while (!mpq_stream_eof(stream)) {
        result->glyphs = realloc(result->glyphs, sizeof(font_glyph_t) * (++result->glyph_count));
        if (result->glyphs == NULL) {
            LOG_FATAL("Failed to allocate memory for font glyphs.");
        }
        font_glyph_t *glyph = &result->glyphs[result->glyph_count - 1];

        mpq_stream_read(stream, &glyph->code, 0, 2);
        mpq_stream_seek(stream, 1, SEEK_CUR); // skip 1 unknown byte
        mpq_stream_read(stream, &glyph->width, 0, 1);
        mpq_stream_read(stream, &glyph->height, 0, 1);
        mpq_stream_seek(stream, 3, SEEK_CUR); // skip 3 unknown bytes
        mpq_stream_read(stream, &glyph->frame_index, 0, 2);

        if (glyph->frame_index == 0xFFFF) {
            // TODO: What is this about? -1?
            glyph->frame_index = 1;
            LOG_WARN("Font '%s' has a glyph with frame index 0xFFFF for glyph %d.", path, result->glyph_count - 1);
        }

        mpq_stream_seek(stream, 4, SEEK_CUR); // skip 4 unknown bytes
    }

    mpq_stream_free(stream);
    free(path_fixed);

    return result;
}

void font_free(font_t *font) {
    free(font->glyphs);
    free(font);
}
