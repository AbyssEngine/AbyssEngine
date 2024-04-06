#ifndef ABYSS_MPQ_HEADER_H
#define ABYSS_MPQ_HEADER_H

#include <stdio.h>
#include <stdint.h>

typedef struct mpq_header_s {
    uint8_t     magic[4];
    uint32_t    header_size;
    uint32_t    archive_size;
    uint16_t    format_version;
    uint16_t    block_size;
    uint32_t    hash_table_offset;
    uint32_t    block_table_offset;
    uint32_t    hash_table_entries;
    uint32_t    block_table_entries;
} mpq_header_t;

void mpq_header_read(FILE* file, const char* mpq_path, mpq_header_t* header);

#endif // ABYSS_MPQ_HEADER_H
