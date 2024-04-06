#ifndef ABYSS_MPQ_H
#define ABYSS_MPQ_H

#include "mpq_hash.h"
#include "mpq_header.h"
#include <string.h>
#include <stdio.h>

typedef struct mpq_s {
    char*               path;
    FILE*               file;
    mpq_header_t        header;
    mpq_hash_entry_t*   hashes;
    int                 hashes_count;
} mpq_t;

mpq_t*  mpq_load    (const char* mpq_path);
void    mpq_free    (mpq_t* mpq);

#endif // ABYSS_MPQ_H
