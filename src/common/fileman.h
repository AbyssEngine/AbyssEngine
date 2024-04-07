#ifndef ABYSS_FILEMAN_H
#define ABYSS_FILEMAN_H

#include "../types/mpq.h"
#include "mpq_stream.h"

typedef struct fileman_file_entry_s {
    uint64_t hash;
    mpq_t *mpq;
} fileman_file_entry_t;

typedef struct fileman_s {
    fileman_file_entry_t *files;
    uint32_t file_count;
    mpq_t **mpqs;
    uint32_t mpq_count;

} fileman_t;

void fileman_init();

void fileman_free();

void fileman_add_mpq(const char *mpq_path);

mpq_stream_t *fileman_load(const char *file_path);

#endif // ABYSS_FILEMAN_H
