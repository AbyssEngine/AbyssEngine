#ifndef ABYSS_MPQ_H
#define ABYSS_MPQ_H

#include "MPQBlock.h"
#include "MPQHash.h"
#include "MPQHeader.h"
#include <stdbool.h>
#include <stdio.h>

struct MPQ {
    char                *path;
    FILE                *file;
    struct MPQHeader     header;
    struct MPQHashEntry *hashes;
    struct MPQBlock     *blocks;
    int                  hashes_count;
};

struct MPQ     *mpq_load(const char *mpq_path);
void            mpq_free(struct MPQ *mpq);
bool            mpq_file_exists(struct MPQ *mpq, const char *file_path);
struct MPQHash *mpq_get_file_hash(const struct MPQ *mpq, const char *file_path);

#endif // ABYSS_MPQ_H
