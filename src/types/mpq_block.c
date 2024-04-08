#include "mpq_block.h"
#include "../common/log.h"
#include "../util/crypto.h"
#include <stdlib.h>

mpq_block_t *mpq_block_read_table(FILE *file, const char *mpq_path, const mpq_header_t *mpq_header) {
    mpq_block_t *result = calloc(sizeof(mpq_block_t), mpq_header->block_table_entries);
    fseek(file, mpq_header->block_table_offset, SEEK_SET);

    uint32_t *block_data = crypto_decrypt_table(file, mpq_header->block_table_entries, "(block table)");

    uint32_t n;
    uint32_t i;

    for (n = 0, i = 0; i < mpq_header->block_table_entries; n += 4, i++) {
        mpq_block_t *entry       = &result[i];
        entry->file_position     = block_data[n];
        entry->size_compressed   = block_data[n + 1];
        entry->size_uncompressed = block_data[n + 2];
        entry->flags             = block_data[n + 3];
    }

    free(block_data);

    return result;
}
