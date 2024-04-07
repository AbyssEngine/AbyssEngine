#ifndef ABYSS_MPQ_HASH_H
#define ABYSS_MPQ_HASH_H

#include "mpq_header.h"
#include <stdint.h>

typedef struct mpq_hash_s {
    uint32_t a;
    uint32_t b;
    uint16_t locale;
    uint16_t platform;
    uint32_t block_index;
} mpq_hash_t;

typedef struct mpq_hash_entry_s {
    uint64_t   key;
    mpq_hash_t hash;
} mpq_hash_entry_t;

mpq_hash_entry_t *mpq_hash_read_table(FILE *file, const char *mpq_path, mpq_header_t *mpq_header);

#endif // ABYSS_MPQ_HASH_H
