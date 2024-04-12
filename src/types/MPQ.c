#include "MPQ.h"
#include "../common/Logging.h"
#include "../util/Crypto.h"
#include <stdlib.h>
#include <string.h>

struct MPQ *mpq_load(const char *mpq_path) {
    LOG_DEBUG("Loading '%s'...", mpq_path);

    struct MPQ *result = malloc(sizeof(struct MPQ));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct MPQ));

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

void mpq_free(struct MPQ *mpq) {
    LOG_DEBUG("Unloading '%s'...", mpq->path);
    fclose(mpq->file);
    free(mpq->path);
    free(mpq->blocks);
    free(mpq->hashes);
    free(mpq);
}

bool mpq_file_exists(struct MPQ *mpq, const char *file_path) { return mpq_get_file_hash(mpq, file_path) != NULL; }

struct MPQHash *mpq_get_file_hash(const struct MPQ *mpq, const char *file_path) {
    const uint64_t file_hash = crypto_hash_file_name(file_path);

    for (uint32_t i = 0; i < mpq->header.hash_table_entries; i++) {
        if (mpq->hashes[i].key != file_hash) {
            continue;
        }

        return &mpq->hashes[i].hash;
    }
    return NULL;
}
