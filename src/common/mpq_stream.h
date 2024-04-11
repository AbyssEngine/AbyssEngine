#ifndef ABYSS_MPQ_STREAM_H
#define ABYSS_MPQ_STREAM_H

#include "../types/mpq.h"
#include <stdint.h>

struct mpq_stream {
    char             *file_name;
    struct mpq       *mpq;
    struct mpq_hash  *hash;
    struct mpq_block *block;
    uint64_t          size;
    uint32_t         *block_offsets;
    uint32_t          block_offset_count;
    void             *data_buffer;
    uint32_t          data_buffer_size;
    uint32_t          position;
    uint32_t          block_index;
};

struct mpq_stream *mpq_stream_create(struct mpq *mpq, const char *file_name);
void               mpq_stream_free(struct mpq_stream *mpq_stream);
uint32_t           mpq_stream_get_size(const struct mpq_stream *mpq_stream);
uint32_t           mpq_stream_read(struct mpq_stream *mpq_stream, void *buffer, uint32_t offset, uint32_t size);
bool               mpq_stream_eof(const struct mpq_stream *mpq_stream);
void               mpq_stream_seek(struct mpq_stream *mpq_stream, int64_t position, int32_t origin);
uint32_t           mpq_stream_tell(const struct mpq_stream *mpq_stream);

void     mpq_stream_load_block_offset(struct mpq_stream *mpq_stream);
uint32_t mpq_stream_read_internal(struct mpq_stream *mpq_stream, void *buffer, uint32_t offset, uint32_t to_read);
void     mpq_stream_buffer_data(struct mpq_stream *mpq_stream);
void    *mpq_stream_load_block(struct mpq_stream *mpq_stream, uint32_t block_index, uint32_t expected_length);
uint32_t mpq_stream_copy(struct mpq_stream *mpq_stream, void *buffer, uint32_t offset, uint32_t position,
                         uint32_t count);
void    *mpq_stream_decompress_multi(void *buffer, uint32_t to_read, uint32_t expected_length);

#endif // ABYSS_MPQ_STREAM_H
