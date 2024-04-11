#include "mpq.h"
#include "../common/log.h"
#include "../util/crypto.h"
#include <stdlib.h>
#include <string.h>

struct mpq *mpq_load(const char *mpq_path) {
    LOG_DEBUG("Loading '%s'...", mpq_path);

    struct mpq *result = malloc(sizeof(struct mpq));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct mpq));

    result->path = strdup(mpq_path);

    result->file = fopen(mpq_path, "rb");
    if (result->file == NULL) {
        LOG_FATAL("Failed to load MPQ '%s'!", mpq_path);
    }

    mpq_header_read(result->file, mpq_path, &result->header);
    result->hashes = mpq_hash_read_table(result->file, &result->header);
    result->blocks = mpq_block_read_table(result->file, &result->header);

    return result;
}

void mpq_free(struct mpq *mpq) {
    LOG_DEBUG("Unloading '%s'...", mpq->path);
    fclose(mpq->file);
    free(mpq->path);
    free(mpq->blocks);
    free(mpq->hashes);
    free(mpq);
}

bool mpq_file_exists(struct mpq *mpq, const char *file_path) { return mpq_get_file_hash(mpq, file_path) != NULL; }

struct mpq_hash *mpq_get_file_hash(const struct mpq *mpq, const char *file_path) {
    const uint64_t file_hash = crypto_hash_file_name(file_path);

    for (uint32_t i = 0; i < mpq->header.hash_table_entries; i++) {
        if (mpq->hashes[i].key != file_hash) {
            continue;
        }

        return &mpq->hashes[i].hash;
    }
    return NULL;
}
