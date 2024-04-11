#ifndef ABYSS_PALETTE_H
#define ABYSS_PALETTE_H

#include <stdint.h>

typedef struct palette_s {
    uint32_t entries[256];
    char    *name;
} palette_t;

void       palette_initialize(void);
void       palette_finalize(void);
palette_t *palette_get(const char *palette_name);

#endif // ABYSS_PALETTE_H
