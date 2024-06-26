#ifndef ABYSS_MPQ_HEADER_H
#define ABYSS_MPQ_HEADER_H

#include <stdint.h>
#include <stdio.h>

typedef struct MPQHeader {
    uint8_t  magic[4];
    uint32_t header_size;
    uint32_t archive_size;
    uint16_t format_version;
    uint16_t block_size;
    uint32_t hash_table_offset;
    uint32_t block_table_offset;
    uint32_t hash_table_entries;
    uint32_t block_table_entries;
} MPQHeader;

void MPQHeader_Read(FILE *file, const char *mpq_path, MPQHeader *header);

#endif // ABYSS_MPQ_HEADER_H
