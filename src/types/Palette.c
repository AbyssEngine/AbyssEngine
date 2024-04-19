#include "Palette.h"
#include "../common/FileManager.h"
#include "../common/Globals.h"
#include "../common/Logging.h"
#include <stdlib.h>
#include <string.h>

struct Palette **palettes;
int              palette_count;

void Palette_Initialize(void) {
    palettes      = calloc(0, sizeof(struct Palette *));
    palette_count = 0;
}

void Palette_Finalize(void) {
    for (int i = 0; i < palette_count; i++) {
        free(palettes[i]->name);
        free(palettes[i]);
    }
    free(palettes);
}

const Palette *Palette_Get(const char *palette_name) {
    for (int i = 0; i < palette_count; i++) {
        if (strcmp(palette_name, palettes[i]->name) != 0) {
            continue;
        }

        return palettes[i];
    }

    Palette *result = malloc(sizeof(Palette));
    FAIL_IF_NULL(result);

    result->name = strdup(palette_name);
    FAIL_IF_NULL(result->name);

    char path_buff[4096];
    memset(path_buff, 0, 4096);
    snprintf(path_buff, 4096, PALETTE_PATH, palette_name);

    MpqStream *stream = FileManager_OpenFile(path_buff);

    if (MpqStream_GetSize(stream) != 256 * 3) {
        LOG_ERROR("Invalid Palette file size. Expected %d but %d was returned.", 256 * 3, MpqStream_GetSize(stream));
    }

    for (int i = 0; i < 256; i++) {
        uint8_t record[3];
        MpqStream_Read(stream, record, 0, 3);
        result->entries[i] = (uint32_t)record[2] << 24 | (uint32_t)record[1] << 16 | (uint32_t)record[0] << 8 | 0xFF;
    }

    MpqStream_Destroy(&stream);

    result->entries[0]          &= 0xFFFFFF00;
    palettes                     = realloc(palettes, sizeof(Palette *) * ++palette_count);
    palettes[palette_count - 1]  = result;

    return result;
}
