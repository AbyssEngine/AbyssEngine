#ifndef ABYSS_MPQ_HASH_H
#define ABYSS_MPQ_HASH_H

#include "MPQHeader.h"
#include <stdint.h>

struct MPQHash {
    uint32_t a;
    uint32_t b;
    uint16_t locale;
    uint16_t platform;
    uint32_t block_index;
};

struct MPQHashEntry {
    uint64_t       key;
    struct MPQHash hash;
};

struct MPQHashEntry *mpq_hash_read_table(FILE *file, const struct MPQHeader *mpq_header);

#endif // ABYSS_MPQ_HASH_H
