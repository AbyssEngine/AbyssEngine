#ifndef ABYSS_MPQ_BLOCK_H
#define ABYSS_MPQ_BLOCK_H

#include "mpq_header.h"
#include <stdint.h>

typedef uint32_t file_flag;

#define FILE_FLAG_IMPLODE       ((file_flag)0x00000100)
#define FILE_FLAG_COMPRESS      ((file_flag)0x00000200)
#define FILE_FLAG_ENCRYPTED     ((file_flag)0x00010000)
#define FILE_FLAG_FIX_KEY       ((file_flag)0x00020000)
#define FILE_FLAG_PATCH_FILE    ((file_flag)0x00100000)
#define FILE_FLAG_SINGLE_UNIT   ((file_flag)0x01000000)
#define FILE_FLAG_DELETE_MARKER ((file_flag)0x02000000)
#define FILE_FLAG_SECTOR_CRC    ((file_flag)0x04000000)
#define FILE_FLAG_EXISTS        ((file_flag)0x80000000)

typedef struct mpq_block_s {
    uint32_t  file_position;
    uint32_t  size_compressed;
    uint32_t  size_uncompressed;
    file_flag flags;
    uint32_t  encryption_seed;
} mpq_block_t;

mpq_block_t *mpq_block_read_table(FILE *file, const char *mpq_path, mpq_header_t *mpq_header);

#endif // ABYSS_MPQ_BLOCK_H
