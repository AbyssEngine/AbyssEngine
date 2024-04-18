#include "FileManager.h"
#include "../util/Crypto.h"
#include "AbyssConfiguration.h"
#include "Logging.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct FileEntry {
    uint64_t    hash;
    struct MPQ *MPQ;
} FileEntry;

typedef struct FileManager {
    struct FileEntry *files;
    uint32_t          file_count;
    struct MPQ      **mpqs;
    uint32_t          mpq_count;
} FileManager;

FileManager *file_manager = NULL;

char *fix_path(const char *path) {
    assert(path != NULL);

    char *result = strdup(path);
    for (char *ch = result; *ch != '\0'; ch++) {
        *ch = (char)tolower((int)*ch);
        if (*ch == '/') {
            *ch = '\\';
        }
    }

    if (result[0] == '\\') {
        const int len = (int)strlen(result);
        for (int i = 0; i < len - 1; i++) {
            result[i] = result[i + 1];
        }
        result[len - 1] = '\0';
    }

    return result;
}

void FileManager_CreateSingleton(void) {
    assert(file_manager == NULL);

    file_manager = malloc(sizeof(FileManager));
    FAIL_IF_NULL(file_manager);

    memset(file_manager, 0, sizeof(FileManager));

    file_manager->mpqs  = calloc(0, sizeof(struct MPQ *));
    file_manager->files = calloc(0, sizeof(FileEntry));

    FAIL_IF_NULL(file_manager->mpqs);
    FAIL_IF_NULL(file_manager->files);

    for (size_t mpq_idx = 0; mpq_idx < AbyssConfiguration_GetMpqCount(); mpq_idx++) {
        FileManager_AddMpq(AbyssConfiguration_GetMpqFileName(mpq_idx));
    }
}

void FileManager_DestroySingleton(void) {
    assert(file_manager != NULL);

    free(file_manager->files);

    for (uint32_t mpq_idx = 0; mpq_idx < file_manager->mpq_count; mpq_idx++) {
        mpq_free(file_manager->mpqs[mpq_idx]);
    }

    free(file_manager->mpqs);
    free(file_manager);

    file_manager = NULL;
}

void FileManager_AddMpq(const char *mpq_path) {
    assert(file_manager != NULL);
    assert(mpq_path != NULL);

    file_manager->mpqs = realloc(file_manager->mpqs, sizeof(struct MPQ *) * ++file_manager->mpq_count);
    file_manager->mpqs[file_manager->mpq_count - 1] = mpq_load(mpq_path);
}

MpqStream *FileManager_OpenFile(const char *file_path) {
    assert(file_manager != NULL);
    assert(file_path != NULL);

    char          *path_fixed = fix_path(file_path);
    const uint64_t file_hash  = crypto_hash_file_name(path_fixed);
    FileEntry     *file_entry = NULL;

    for (uint32_t i = 0; i < file_manager->file_count; i++) {
        if (file_manager->files[i].hash != file_hash) {
            continue;
        }
        file_entry = &file_manager->files[i];
        break;
    }

    if (file_entry == NULL) {
        for (uint32_t mpq_idx = 0; mpq_idx < file_manager->mpq_count; mpq_idx++) {
            if (!mpq_file_exists(file_manager->mpqs[mpq_idx], path_fixed)) {
                continue;
            }

            file_manager->files = realloc(file_manager->files, sizeof(FileEntry) * ++file_manager->file_count);
            file_entry          = &file_manager->files[file_manager->file_count - 1];

            file_entry->hash = file_hash;
            file_entry->MPQ  = file_manager->mpqs[mpq_idx];
        }

        if (file_entry == NULL) {
            LOG_FATAL("Could not find '%s'", file_path);
        }
    }

    MpqStream *MpqStream = MpqStream_Create(file_entry->MPQ, path_fixed);
    free(path_fixed);
    return MpqStream;
}
