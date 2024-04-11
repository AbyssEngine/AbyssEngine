#include "mpq_block.h"
#include "../common/log.h"
#include "../util/crypto.h"
#include <stdlib.h>
#include <string.h>

struct mpq_block *mpq_block_read_table(FILE *file, const mpq_header_t *mpq_header) {
    struct mpq_block *result = calloc(sizeof(struct mpq_block), mpq_header->block_table_entries);
    fseek(file, mpq_header->block_table_offset, SEEK_SET);

    uint32_t *block_data = crypto_decrypt_table(file, mpq_header->block_table_entries, "(block table)");

    uint32_t n;
    uint32_t i;

    for (n = 0, i = 0; i < mpq_header->block_table_entries; n += 4, i++) {
        struct mpq_block *entry  = &result[i];
        entry->file_position     = block_data[n];
        entry->size_compressed   = block_data[n + 1];
        entry->size_uncompressed = block_data[n + 2];
        entry->flags             = block_data[n + 3];
    }

    free(block_data);

    return result;
}
void mpq_block_calculate_encryption_seed(struct mpq_block *block, const char *file_name) {
    const char *name       = strrchr(file_name, '\\') + 1;
    uint32_t    seed       = crypto_hash_string(name, 3);
    block->encryption_seed = (seed + block->file_position) ^ block->size_uncompressed;
}
