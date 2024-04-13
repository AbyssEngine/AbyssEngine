#include "FileManager.h"
#include "../util/Crypto.h"
#include "AbyssConfiguration.h"
#include "Logging.h"
#include <ctype.h>
#include <stdlib.h>

struct FileManager *file_manager;

char *fix_path(const char *path) {
    char *result = strdup(path);
    for (char *ch = result; *ch != '\0'; ch++) {
        *ch = (char)tolower((int)*ch);
        if (*ch == '/') {
            *ch = '\\';
        }
    }

    if (result[0] == '\\') {
        const int len = strlen(result);
        for (int i = 0; i < len - 1; i++) {
            result[i] = result[i + 1];
        }
        result[len - 1] = '\0';
    }

    return result;
}

void file_manager_init(void) {
    file_manager = malloc(sizeof(struct FileManager));
    FAIL_IF_NULL(file_manager);

    memset(file_manager, 0, sizeof(struct FileManager));

    file_manager->mpqs  = calloc(0, sizeof(struct MPQ *));
    file_manager->files = calloc(0, sizeof(struct FileEntry));

    FAIL_IF_NULL(file_manager->mpqs);
    FAIL_IF_NULL(file_manager->files);

    for (int mpq_idx = 0; mpq_idx < abyss_configuration.num_mpqs; mpq_idx++) {
        file_manager_add_mpq(abyss_configuration.mpqs[mpq_idx]);
    }
}

void file_manager_free(void) {
    free(file_manager->files);

    for (uint32_t mpq_idx = 0; mpq_idx < file_manager->mpq_count; mpq_idx++) {
        mpq_free(file_manager->mpqs[mpq_idx]);
    }
    free(file_manager->mpqs);

    free(file_manager);
}

void file_manager_add_mpq(const char *mpq_path) {
    file_manager->mpqs = realloc(file_manager->mpqs, sizeof(struct MPQ) * ++file_manager->mpq_count);
    file_manager->mpqs[file_manager->mpq_count - 1] = mpq_load(mpq_path);
}

struct MpqStream *file_manager_load(const char *file_path) {

    char             *path_fixed = fix_path(file_path);
    const uint64_t    file_hash  = crypto_hash_file_name(path_fixed);
    struct FileEntry *file_entry = NULL;

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

            file_manager->files = realloc(file_manager->files, sizeof(struct FileEntry) * ++file_manager->file_count);
            file_entry          = &file_manager->files[file_manager->file_count - 1];

            file_entry->hash = file_hash;
            file_entry->MPQ  = file_manager->mpqs[mpq_idx];
        }

        if (file_entry == NULL) {
            LOG_FATAL("Could not find '%s'", file_path);
        }
    }

    struct MpqStream *MpqStream = mpq_stream_create(file_entry->MPQ, path_fixed);
    free(path_fixed);
    return MpqStream;
}
