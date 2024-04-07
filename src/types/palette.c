#include "palette.h"
#include "../common/fileman.h"
#include "../common/globals.h"
#include "../common/log.h"
#include <string.h>

palette_t **palettes;
int         palette_count;

void palette_initialize() {
    palettes      = calloc(0, sizeof(palette_t *));
    palette_count = 0;
}

void palette_finalize() {
    for (int i = 0; i < palette_count; i++) {
        free(palettes[i]->name);
        free(palettes[i]);
    }
    free(palettes);
}

palette_t *palette_get(const char *palette_name) {
    for (int i = 0; i < palette_count; i++) {
        if (strcmp(palette_name, palettes[i]->name)) {
            continue;
        }

        return palettes[i];
    }

    palette_t *result = malloc(sizeof(palette_t));
    result->name      = strdup(palette_name);
    char *path_buff   = malloc(4096);

    memset(path_buff, 0, 4096);
    snprintf(path_buff, 4096, PALETTE_PATH, palette_name);

    mpq_stream_t *stream = fileman_load(path_buff);

    free(path_buff);

    if (mpq_stream_get_size(stream) != 256 * 3) {
        LOG_ERROR("Invalid palette file size. Expected %d but %d was returned.", 256 * 3, mpq_stream_get_size(stream));
    }

    uint8_t record[3];

    for (int i = 0; i < 256; i++) {
        mpq_stream_read(stream, record, 0, 3);
        result->entries[i] =
            ((uint32_t)record[2] << 24) | ((uint32_t)record[1] << 16) | ((uint32_t)record[0] << 8) | 0xFF;
    }

    mpq_stream_free(stream);

    result->entries[0]          &= 0xFFFFFF00;
    palettes                     = realloc(palettes, sizeof(palette_t) * ++palette_count);
    palettes[palette_count - 1]  = result;

    return result;
}
