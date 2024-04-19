#ifndef ABYSS_PALETTE_H
#define ABYSS_PALETTE_H

#include <stdint.h>

typedef struct Palette {
    uint32_t entries[256];
    char    *name;
} Palette;

void           Palette_Initialize(void);
void           Palette_Finalize(void);
const Palette *Palette_Get(const char *palette_name);

#endif // ABYSS_PALETTE_H
