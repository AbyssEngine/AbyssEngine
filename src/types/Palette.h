#ifndef ABYSS_PALETTE_H
#define ABYSS_PALETTE_H

#include <stdint.h>

struct Palette {
    uint32_t entries[256];
    char    *name;
};

void            palette_initialize(void);
void            palette_finalize(void);
struct Palette *palette_get(const char *palette_name);

#endif // ABYSS_PALETTE_H
