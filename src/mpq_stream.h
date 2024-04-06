#ifndef ABYSS_MPQ_STREAM_H
#define ABYSS_MPQ_STREAM_H

#include "mpq.h"
#include <stdint.h>

typedef struct mpq_stream_s {
    char*           file_name;
    mpq_t*          mpq;
    mpq_hash_t*     hash;
    mpq_block_t*    block;
    uint64_t        size;
    uint32_t*       block_offsets;
    uint32_t        block_offset_count;
} mpq_stream_t;


mpq_stream_t*   mpq_stream_create               (mpq_t* mpq, const char* file_name);
void            mpq_stream_free                 (mpq_stream_t* mpq_stream);
void            mpq_stream_load_block_offset    (mpq_stream_t* mpq_stream);

#endif // ABYSS_MPQ_STREAM_H
