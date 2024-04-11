#ifndef ABYSS_FILEMAN_H
#define ABYSS_FILEMAN_H

#include "../types/mpq.h"
#include "mpq_stream.h"

struct fileman_file_entry {
    uint64_t    hash;
    struct mpq *mpq;
};

struct fileman {
    struct fileman_file_entry *files;
    uint32_t                   file_count;
    struct mpq               **mpqs;
    uint32_t                   mpq_count;
};

void               fileman_init(void);
void               fileman_free(void);
void               fileman_add_mpq(const char *mpq_path);
struct mpq_stream *fileman_load(const char *file_path);

#endif // ABYSS_FILEMAN_H
