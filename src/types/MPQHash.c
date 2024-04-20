#include "MPQHash.h"
#include "../util/Crypto.h"
#include <stdlib.h>

MPQHashEntry *MPQ_LoadHashTable(FILE *file, const MPQHeader *mpq_header) {
    MPQHashEntry *result = calloc(sizeof(MPQHashEntry), mpq_header->hash_table_entries);
    fseek(file, mpq_header->hash_table_offset, SEEK_SET);

    uint32_t *hash_data = crypto_decrypt_table(file, mpq_header->hash_table_entries, "(hash table)");

    uint32_t n;
    uint32_t i;
    for (n = 0, i = 0; i < mpq_header->hash_table_entries; n += 4, i++) {
        MPQHash *entry = &result[i].hash;

        entry->a           = hash_data[n];
        entry->b           = hash_data[n + 1];
        entry->locale      = (uint16_t)(hash_data[n + 2] >> 16);
        entry->platform    = (uint16_t)(hash_data[n + 2] & 0xFFFF);
        entry->block_index = hash_data[n + 3];

        result[i].key = (uint64_t)entry->a << 32 | (uint64_t)entry->b;
    }

    free(hash_data);

    return result;
}
