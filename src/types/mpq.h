#ifndef ABYSS_MPQ_H
#define ABYSS_MPQ_H

#include "mpq_block.h"
#include "mpq_hash.h"
#include "mpq_header.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct mpq_s {
    char             *path;
    FILE             *file;
    mpq_header_t      header;
    mpq_hash_entry_t *hashes;
    mpq_block_t      *blocks;
    int               hashes_count;
} mpq_t;

mpq_t      *mpq_load(const char *mpq_path);
void        mpq_free(mpq_t *mpq);
bool        mpq_file_exists(mpq_t *mpq, const char *file_path);
mpq_hash_t *mpq_get_file_hash(const mpq_t *mpq, const char *file_path);

#endif // ABYSS_MPQ_H
