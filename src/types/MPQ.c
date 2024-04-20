#include "MPQ.h"
#include "../common/Logging.h"
#include "../util/Crypto.h"
#include "../util/Mutex.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct MPQ {
    char                *path;
    FILE                *file;
    struct MPQHeader     header;
    struct MPQHashEntry *hashes;
    struct MPQBlock     *blocks;
    int                  hashes_count;
    struct Mutex        *mutex;
};

struct MPQBlock *MPQ__LoadBlocks(FILE *file, const MPQ *mpq);

struct MPQ *MPQ_Load(const char *mpq_path) {
    LOG_DEBUG("Loading '%s'...", mpq_path);

    struct MPQ *result = malloc(sizeof(struct MPQ));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct MPQ));

    result->path = strdup(mpq_path);

    result->file = fopen(mpq_path, "rb");
    if (result->file == NULL) {
        LOG_FATAL("Failed to load MPQ '%s'!", mpq_path);
    }

    MPQHeader_Read(result->file, mpq_path, &result->header);
    result->hashes = MPQ_LoadHashTable(result->file, &result->header);
    result->blocks = MPQ__LoadBlocks(result->file, result);

    result->mutex = Mutex_Create();

    return result;
}

void MPQ_Destroy(MPQ *mpq) {
    LOG_DEBUG("Unloading '%s'...", mpq->path);

    Mutex_Lock(mpq->mutex); // Ensure we have don't have dangling access open somewhere else...

    fclose(mpq->file);
    free(mpq->path);
    free(mpq->blocks);
    free(mpq->hashes);
    Mutex_Destroy(&mpq->mutex);

    free(mpq);
}

bool MPQ_FileExists(const MPQ *mpq, const char *file_path) { return MPQ_GetFileHash(mpq, file_path) != NULL; }

struct MPQHash *MPQ_GetFileHash(const MPQ *mpq, const char *file_path) {
    const uint64_t file_hash = crypto_hash_file_name(file_path);

    for (uint32_t i = 0; i < mpq->header.hash_table_entries; i++) {
        if (mpq->hashes[i].key != file_hash) {
            continue;
        }

        return &mpq->hashes[i].hash;
    }
    return NULL;
}

struct MPQBlock *MPQ_GetBlock(const MPQ *mpq, uint32_t block_index) {
    assert(mpq != NULL);

    if (block_index >= mpq->header.hash_table_entries) {
        LOG_FATAL("Invalid block index!");
    }

    return &mpq->blocks[block_index];
}

uint64_t MPQ_GetBlockSize(const MPQ *mpq) {
    assert(mpq != NULL);

    return 0x200 << mpq->header.block_size;
}

FILE *MPQ_AcquireFileHandle(MPQ *mpq) {
    Mutex_Lock(mpq->mutex);
    return mpq->file;
}

void MPQ_ReleaseFileHandle(MPQ *mpq) { Mutex_Unlock(mpq->mutex); }

struct MPQBlock *MPQ__LoadBlocks(FILE *file, const MPQ *mpq) {
    struct MPQBlock *result = calloc(sizeof(struct MPQBlock), mpq->header.block_table_entries);
    fseek(file, mpq->header.block_table_offset, SEEK_SET);

    uint32_t *block_data = crypto_decrypt_table(file, mpq->header.block_table_entries, "(block table)");

    uint32_t n;
    uint32_t i;

    for (n = 0, i = 0; i < mpq->header.block_table_entries; n += 4, i++) {
        struct MPQBlock *entry   = &result[i];
        entry->file_position     = block_data[n];
        entry->size_compressed   = block_data[n + 1];
        entry->size_uncompressed = block_data[n + 2];
        entry->flags             = block_data[n + 3];
    }

    free(block_data);

    return result;
}
