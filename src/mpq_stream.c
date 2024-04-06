#include "mpq_stream.h"

#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

mpq_stream_t* mpq_stream_create(mpq_t* mpq, const char* file_name) {
    LOG_DEBUG("Loading '%s'", file_name);
    mpq_stream_t* result = malloc(sizeof(mpq_stream_t));
    memset(result, 0, sizeof(mpq_stream_t));
    result->mpq = mpq;
    result->file_name = strdup(file_name);

    result->hash = mpq_get_file_hash(mpq, file_name);
    if (result->hash == NULL) {
        LOG_FATAL("Failed to load '%s'!", file_name);
    }
    uint32_t block_index = result->hash->block_index;
    
    if (block_index >= mpq->header.hash_table_entries) {
        LOG_FATAL("Invalid block index for '%s'!", file_name);
    }
    result->block = &mpq->blocks[block_index];

    if (result->block->flags & FILE_FLAG_FIX_KEY) {
        // TODO: The thing
        LOG_FATAL("TODO: Recalculate encryption seed for block based on file name.");
    }
    
    result->size = 0x200 << mpq->header.block_size;
    
    if (result->block->flags & FILE_FLAG_PATCH_FILE) {
        LOG_FATAL("TODO: Patch Files");
    }
    
    if (((result->block->flags & FILE_FLAG_COMPRESS) || (result->block->flags & FILE_FLAG_IMPLODE)) 
         && !(result->block->flags & FILE_FLAG_SINGLE_UNIT)) {
        mpq_stream_load_block_offset(result);
    }

    return result;
}

void mpq_stream_load_block_offset(mpq_stream_t* mpq_stream) {
    fseek(mpq_stream->mpq->file, mpq_stream->block->file_position, SEEK_SET);
    
    mpq_stream->block_offset_count = ((mpq_stream->block->size_uncompressed + mpq_stream->size - 1) / mpq_stream->size) + 1;
    
    uint32_t offset_file_load_size = sizeof(uint32_t) * mpq_stream->block_offset_count;
        
    mpq_stream->block_offsets = malloc(offset_file_load_size);
    memset(mpq_stream->block_offsets, 0, offset_file_load_size);
    if (fread(mpq_stream->block_offsets, sizeof(uint32_t), mpq_stream->block_offset_count, mpq_stream->mpq->file) != mpq_stream->block_offset_count) {
        LOG_FATAL("Failed to load block offsets for '%s'", mpq_stream->file_name);         
    }
    
    if (mpq_stream->block->flags & FILE_FLAG_ENCRYPTED) {
        LOG_FATAL("TODO: Encrypted flag on block offset load");
    }
}

void mpq_stream_free(mpq_stream_t* mpq_stream) {
    if (mpq_stream->block_offset_count > 0) {
        free(mpq_stream->block_offsets);
    }
    free(mpq_stream->file_name);
    free(mpq_stream);
}

