#include "fileman.h"
#include "../util/crypto.h"
#include "config.h"
#include "log.h"
#include <ctype.h>
#include <stdlib.h>

struct fileman *fileman;

char *fix_path(const char *path) {
    char *result = strdup(path);
    for (char *ch = result; *ch != '\0'; ch++) {
        *ch = tolower(*ch);
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

void fileman_init(void) {
    fileman = malloc(sizeof(struct fileman));
    FAIL_IF_NULL(fileman);

    memset(fileman, 0, sizeof(struct fileman));

    fileman->mpqs  = calloc(0, sizeof(struct mpq *));
    fileman->files = calloc(0, sizeof(struct fileman_file_entry));

    FAIL_IF_NULL(fileman->mpqs);
    FAIL_IF_NULL(fileman->files);

    for (int mpq_idx = 0; mpq_idx < config.num_mpqs; mpq_idx++) {
        fileman_add_mpq(config.mpqs[mpq_idx]);
    }
}

void fileman_free(void) {
    free(fileman->files);

    for (uint32_t mpq_idx = 0; mpq_idx < fileman->mpq_count; mpq_idx++) {
        mpq_free(fileman->mpqs[mpq_idx]);
    }
    free(fileman->mpqs);

    free(fileman);
}

void fileman_add_mpq(const char *mpq_path) {
    fileman->mpqs                         = realloc(fileman->mpqs, sizeof(struct mpq) * ++fileman->mpq_count);
    fileman->mpqs[fileman->mpq_count - 1] = mpq_load(mpq_path);
}

struct mpq_stream *fileman_load(const char *file_path) {

    char                      *path_fixed = fix_path(file_path);
    const uint64_t             file_hash  = crypto_hash_file_name(path_fixed);
    struct fileman_file_entry *file_entry = NULL;

    for (uint32_t i = 0; i < fileman->file_count; i++) {
        if (fileman->files[i].hash != file_hash) {
            continue;
        }
        file_entry = &fileman->files[i];
        break;
    }

    if (file_entry == NULL) {
        for (uint32_t mpq_idx = 0; mpq_idx < fileman->mpq_count; mpq_idx++) {
            if (!mpq_file_exists(fileman->mpqs[mpq_idx], path_fixed)) {
                continue;
            }

            fileman->files = realloc(fileman->files, sizeof(struct fileman_file_entry) * ++fileman->file_count);
            file_entry     = &fileman->files[fileman->file_count - 1];

            file_entry->hash = file_hash;
            file_entry->mpq  = fileman->mpqs[mpq_idx];
        }

        if (file_entry == NULL) {
            LOG_FATAL("Could not find '%s'", file_path);
        }
    }

    struct mpq_stream *mpq_stream = mpq_stream_create(file_entry->mpq, path_fixed);
    free(path_fixed);
    return mpq_stream;
}
