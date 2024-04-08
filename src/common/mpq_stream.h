#ifndef ABYSS_MPQ_STREAM_H
#define ABYSS_MPQ_STREAM_H

#include "../types/mpq.h"
#include <stdint.h>

typedef struct mpq_stream_s {
    char        *file_name;
    mpq_t       *mpq;
    mpq_hash_t  *hash;
    mpq_block_t *block;
    uint64_t     size;
    uint32_t    *block_offsets;
    uint32_t     block_offset_count;
    void        *data_buffer;
    uint32_t     data_buffer_size;
    uint32_t     position;
    uint32_t     block_index;
} mpq_stream_t;

mpq_stream_t *mpq_stream_create(mpq_t *mpq, const char *file_name);
void          mpq_stream_free(mpq_stream_t *mpq_stream);
uint32_t      mpq_stream_get_size(const mpq_stream_t *mpq_stream);
uint32_t      mpq_stream_read(mpq_stream_t *mpq_stream, void *buffer, uint32_t offset, uint32_t size);
void          mpq_stream_load_block_offset(mpq_stream_t *mpq_stream);
uint32_t      mpq_stream_read_internal(mpq_stream_t *mpq_stream, void *buffer, uint32_t offset, uint32_t to_read);
void          mpq_stream_buffer_data(mpq_stream_t *mpq_stream);
void         *mpq_stream_load_block(mpq_stream_t *mpq_stream, uint32_t block_index, uint32_t expected_length);
uint32_t mpq_stream_copy(mpq_stream_t *mpq_stream, void *buffer, uint32_t offset, uint32_t position, uint32_t count);
void *mpq_stream_decompress_multi(mpq_stream_t *mpq_stream, void *buffer, uint32_t to_read, uint32_t expected_length);
 
#endif // ABYSS_MPQ_STREAM_H
