#ifndef ABYSS_MPQ_H
#define ABYSS_MPQ_H

#include "mpq_block.h"
#include "mpq_hash.h"
#include "mpq_header.h"
#include <stdbool.h>
#include <stdio.h>

struct mpq {
    char             *path;
    FILE             *file;
    mpq_header_t      header;
    mpq_hash_entry_t *hashes;
    struct mpq_block *blocks;
    int               hashes_count;
};

struct mpq      *mpq_load(const char *mpq_path);
void             mpq_free(struct mpq *mpq);
bool             mpq_file_exists(struct mpq *mpq, const char *file_path);
struct mpq_hash *mpq_get_file_hash(const struct mpq *mpq, const char *file_path);

#endif // ABYSS_MPQ_H
