#ifndef ABYSS_MPQ_HASH_H
#define ABYSS_MPQ_HASH_H

#include "MPQHeader.h"
#include <stdint.h>

typedef struct MPQHash {
    uint32_t a;
    uint32_t b;
    uint16_t locale;
    uint16_t platform;
    uint32_t block_index;
} MPQHash;

typedef struct MPQHashEntry {
    uint64_t key;
    MPQHash  hash;
} MPQHashEntry;

MPQHashEntry *MPQ_LoadHashTable(FILE *file, const MPQHeader *mpq_header);

#endif // ABYSS_MPQ_HASH_H
