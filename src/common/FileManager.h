#ifndef ABYSS_FILEMAN_H
#define ABYSS_FILEMAN_H

#include "../types/MPQ.h"
#include "MpqStream.h"

struct FileEntry {
    uint64_t    hash;
    struct MPQ *MPQ;
};

struct FileManager {
    struct FileEntry *files;
    uint32_t          file_count;
    struct MPQ      **mpqs;
    uint32_t          mpq_count;
};

void              file_manager_init(void);
void              file_manager_free(void);
void              file_manager_add_mpq(const char *mpq_path);
struct MpqStream *file_manager_load(const char *file_path);

#endif // ABYSS_FILEMAN_H
