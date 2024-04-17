#include "MpqStream.h"

#include "../util/Crypto.h"
#include "../util/Huffman.h"
#include "../util/Implode.h"
#include "../util/WavDecompress.h"
#include "Logging.h"
#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPRESSION_TYPE_HUFFMAN                 0x01
#define COMPRESSION_TYPE_ZLIB_DEFLATE            0x02
#define COMPRESSION_TYPE_PKLIB_IMPLODE           0x08
#define COMPRESSION_TYPE_BZIP2                   0x10
#define COMPRESSION_TYPE_LZMA                    0x12
#define COMPRESSION_TYPE_SPARSE_THEN_ZLIB        0x22
#define COMPRESSION_TYPE_SPARSE_THEN_BZIP2       0x30
#define COMPRESSION_TYPE_IMA_ADPCM_MONO          0x40
#define COMPRESSION_TYPE_HUFFMAN_THEN_ADPCM_MONO 0x41
#define COMPRESSION_TYPE_IMA_ADPCM_STEREO        0x80
#define COMPRESSION_TYPE_HUFFMAN_THEN_WAV_STEREO 0x81

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

struct pk_info {
    void    *buff_in;
    void    *buff_out;
    uint32_t out_pos;
    uint32_t in_pos;
    uint32_t to_read;
    uint32_t to_write;
};

unsigned int explode_read(char *buf, unsigned int *size, void *param) {
    struct pk_info *pk_info = param;
    const uint32_t  to_read = MIN(*size, pk_info->to_read);
    memcpy(buf, (char *)pk_info->buff_in + pk_info->in_pos, to_read);
    pk_info->in_pos  += to_read;
    pk_info->to_read -= to_read;

    *size = to_read;
    return *size;
}

void explode_write(char *buf, unsigned int *size, void *param) {
    struct pk_info *pk_info = param;

    if (*size > pk_info->to_write) {
        LOG_ERROR("Attempted to write past end of stream for PkWare Explode decompression.");
    }

    memcpy((char *)pk_info->buff_out + pk_info->out_pos, buf, *size);
    pk_info->out_pos  += *size;
    pk_info->to_write -= *size;
}

struct MpqStream *mpq_stream_create(struct MPQ *mpq, const char *file_name) {
    LOG_DEBUG("Loading '%s'", file_name);

    struct MpqStream *result = malloc(sizeof(struct MpqStream));
    FAIL_IF_NULL(result);

    memset(result, 0, sizeof(struct MpqStream));

    result->mpq         = mpq;
    result->file_name   = strdup(file_name);
    result->block_index = 0xFFFFFFFF;

    result->hash = mpq_get_file_hash(mpq, file_name);
    if (result->hash == NULL) {
        LOG_FATAL("Failed to load '%s'!", file_name);
    }
    const uint32_t block_index = result->hash->block_index;

    if (block_index >= mpq->header.hash_table_entries) {
        LOG_FATAL("Invalid block index for '%s'!", file_name);
    }
    result->block = &mpq->blocks[block_index];

    if (result->block->flags & FILE_FLAG_FIX_KEY) {
        mpq_block_calculate_encryption_seed(result->block, file_name);
    }

    result->size = 0x200 << mpq->header.block_size;

    if (result->block->flags & FILE_FLAG_PATCH_FILE) {
        LOG_FATAL("TODO: Patch Files");
    }

    if (((result->block->flags & FILE_FLAG_COMPRESS) || (result->block->flags & FILE_FLAG_IMPLODE)) &&
        !(result->block->flags & FILE_FLAG_SINGLE_UNIT)) {
        mpq_stream_load_block_offset(result);
    }

    return result;
}

void mpq_stream_load_block_offset(struct MpqStream *mpq_stream) {
    fseek(mpq_stream->mpq->file, mpq_stream->block->file_position, SEEK_SET);

    mpq_stream->block_offset_count =
        (uint32_t)((mpq_stream->block->size_uncompressed + mpq_stream->size - 1) / mpq_stream->size) + 1;

    const uint32_t offset_file_load_size = sizeof(uint32_t) * mpq_stream->block_offset_count;

    mpq_stream->block_offsets = malloc(offset_file_load_size);
    FAIL_IF_NULL(mpq_stream->block_offsets);

    memset(mpq_stream->block_offsets, 0, offset_file_load_size);

    if (fread(mpq_stream->block_offsets, sizeof(uint32_t), mpq_stream->block_offset_count, mpq_stream->mpq->file) !=
        mpq_stream->block_offset_count) {
        LOG_FATAL("Failed to load block offsets for '%s'", mpq_stream->file_name);
    }

    if (mpq_stream->block->flags & FILE_FLAG_ENCRYPTED) {
        crypto_decrypt(mpq_stream->block_offsets, mpq_stream->block_offset_count,
                       mpq_stream->block->encryption_seed - 1);

        uint64_t block_pos_size = mpq_stream->block_offset_count << 2;

        if (mpq_stream->block_offsets[0] != block_pos_size) {
            LOG_FATAL("Decryption of MPQ failed");
        }

        if (mpq_stream->block_offsets[1] > mpq_stream->block->size_uncompressed + block_pos_size) {
            LOG_FATAL("Decryption of MPQ failed");
        }
    }
}

uint32_t mpq_stream_read(struct MpqStream *mpq_stream, void *buffer, uint32_t offset, const uint32_t size) {
    if (mpq_stream->block->flags & FILE_FLAG_SINGLE_UNIT) {
        LOG_FATAL("TODO: Single unit loads");
    }

    uint32_t to_read    = size;
    uint32_t read_total = 0;

    while (to_read > 0) {
        const uint32_t read = mpq_stream_read_internal(mpq_stream, (char *)buffer, offset, to_read);
        if (read == 0) {
            break;
        }

        read_total += read;
        offset     += read;
        to_read    -= read;
    }

    return read_total;
}

uint32_t mpq_stream_read_internal(struct MpqStream *mpq_stream, void *buffer, const uint32_t offset,
                                  const uint32_t to_read) {
    mpq_stream_buffer_data(mpq_stream);
    const uint32_t local_position = mpq_stream->position % mpq_stream->size;
    return mpq_stream_copy(mpq_stream, buffer, offset, local_position, to_read);
}

void mpq_stream_buffer_data(struct MpqStream *mpq_stream) {
    const uint32_t block_index = mpq_stream->position / mpq_stream->size;

    if (block_index == mpq_stream->block_index) {
        return;
    }

    if (mpq_stream->data_buffer != NULL) {
        free(mpq_stream->data_buffer);
    }

    const uint32_t expected_length = (uint32_t)MIN(
        (uint64_t)mpq_stream->block->size_uncompressed - (block_index * mpq_stream->size), mpq_stream->size);
    mpq_stream->data_buffer      = mpq_stream_load_block(mpq_stream, block_index, expected_length);
    mpq_stream->data_buffer_size = expected_length;
    mpq_stream->block_index      = block_index;
}

void mpq_stream_free(struct MpqStream *mpq_stream) {
    if (mpq_stream->block_offsets != NULL) {
        free(mpq_stream->block_offsets);
    }
    if (mpq_stream->data_buffer != NULL) {
        free(mpq_stream->data_buffer);
    }
    free(mpq_stream->file_name);
    free(mpq_stream);
}

uint32_t mpq_stream_copy(struct MpqStream *mpq_stream, void *buffer, const uint32_t offset, const uint32_t position,
                         const uint32_t count) {
    const uint32_t bytes_to_copy = MIN(mpq_stream->data_buffer_size - position, count);
    if (bytes_to_copy <= 0) {
        //        LOG_FATAL("Tried reading past end of stream!");
        return 0;
    }

    memcpy((char *)buffer + offset, (char *)mpq_stream->data_buffer + position, bytes_to_copy);
    mpq_stream->position += bytes_to_copy;

    return bytes_to_copy;
}

void *mpq_stream_load_block(struct MpqStream *mpq_stream, const uint32_t block_index, const uint32_t expected_length) {
    uint32_t offset;
    uint32_t to_read;

    if ((mpq_stream->block->flags & FILE_FLAG_COMPRESS) || (mpq_stream->block->flags & FILE_FLAG_IMPLODE)) {
        offset  = mpq_stream->block_offsets[block_index];
        to_read = mpq_stream->block_offsets[block_index + 1] - offset;
    } else {
        offset  = block_index * (uint32_t)mpq_stream->size;
        to_read = expected_length;
    }

    offset += mpq_stream->block->file_position;

    void *data = malloc(to_read);
    FAIL_IF_NULL(data);

    fseek(mpq_stream->mpq->file, offset, SEEK_SET);
    if (fread(data, to_read, 1, mpq_stream->mpq->file) != 1) {
        LOG_FATAL("Error loading file block data.");
    }

    if ((mpq_stream->block->flags & FILE_FLAG_ENCRYPTED) && mpq_stream->block->size_uncompressed > 3) {
        if (mpq_stream->block->encryption_seed == 0) {
            LOG_FATAL("Unable to determine encryption key for file block load.");
        }

        crypto_decrypt_bytes(data, to_read, block_index + mpq_stream->block->encryption_seed);
    }

    if ((mpq_stream->block->flags & FILE_FLAG_COMPRESS) && (to_read != expected_length)) {
        if (mpq_stream->block->flags & FILE_FLAG_SINGLE_UNIT) {
            LOG_FATAL("TODO: PK Decompression");
        }

        return mpq_stream_decompress_multi(data, to_read, expected_length);
    }

    if ((mpq_stream->block->flags & FILE_FLAG_IMPLODE) && (to_read != expected_length)) {
        LOG_FATAL("TODO: PK decompress");
    }

    return data;
}

void *mpq_stream_decompress_multi(void *buffer, const uint32_t to_read, const uint32_t expected_length) {
    const uint8_t compression_type = ((uint8_t *)buffer)[0];

    switch (compression_type) {
    case COMPRESSION_TYPE_ZLIB_DEFLATE: {
        void *out_buffer = malloc(expected_length + 1);
        FAIL_IF_NULL(out_buffer);

        memset(out_buffer, 0, expected_length + 1);
        z_stream inflate_stream;
        inflate_stream.zalloc    = Z_NULL;
        inflate_stream.zfree     = Z_NULL;
        inflate_stream.opaque    = Z_NULL;
        inflate_stream.avail_in  = to_read - 1;
        inflate_stream.next_in   = (Bytef *)buffer + 1;
        inflate_stream.avail_out = expected_length;
        inflate_stream.next_out  = (Bytef *)out_buffer;
        inflateInit(&inflate_stream);
        inflate(&inflate_stream, Z_NO_FLUSH);
        inflateEnd(&inflate_stream);

        if (inflate_stream.msg != NULL) {
            LOG_FATAL("ZLIB Deflate Error: %s", inflate_stream.msg);
        }

        free(buffer);
        return out_buffer;
    }
    case COMPRESSION_TYPE_PKLIB_IMPLODE: {
        void *out_buffer = malloc(expected_length + 1);
        FAIL_IF_NULL(out_buffer);

        struct pk_info pk_info;
        memset(&pk_info, 0, sizeof(struct pk_info));
        pk_info.buff_out = out_buffer;
        pk_info.buff_in  = (char *)buffer + 1;
        pk_info.to_read  = to_read - 1;
        pk_info.to_write = expected_length;

        char *work_buff = malloc(15000);
        FAIL_IF_NULL(work_buff);

        memset(work_buff, 0, 15000);
        memset(out_buffer, 0, expected_length + 1);
        const int pk_result = explode(explode_read, explode_write, work_buff, &pk_info);

        if (pk_result != CMP_NO_ERROR) {
            LOG_FATAL("Failed to decompress using PkWare Explode: %d.", pk_result);
        }

        if (pk_info.out_pos != expected_length) {
            LOG_FATAL("Decompression failed. Expected %d bytes but got %d instead!", expected_length, pk_info.out_pos);
        }

        free(work_buff);
        free(buffer);
        return out_buffer;
    }
    case COMPRESSION_TYPE_HUFFMAN_THEN_ADPCM_MONO: {
        uint32_t huffman_buffer_size = 0;
        uint8_t *huffman_buffer      = huffman_decompress((uint8_t *)buffer + 1, to_read - 1, &huffman_buffer_size);

        uint32_t wav_size   = 0;
        uint8_t *wav_buffer = wav_decompress(huffman_buffer, huffman_buffer_size, 1, &wav_size);
        if (wav_size != expected_length) {
            LOG_FATAL("Decompression failed: Expected WAV buffer of %d bytes, but got %d instead!", expected_length,
                      wav_size);
        }
        free(huffman_buffer);
        free(buffer);
        return wav_buffer;
    }
    case COMPRESSION_TYPE_HUFFMAN_THEN_WAV_STEREO: {
        uint32_t huffman_buffer_size = 0;
        uint8_t *huffman_buffer      = huffman_decompress((uint8_t *)buffer + 1, to_read - 1, &huffman_buffer_size);

        uint32_t wav_size   = 0;
        uint8_t *wav_buffer = wav_decompress(huffman_buffer, huffman_buffer_size, 2, &wav_size);
        if (wav_size != expected_length) {
            LOG_FATAL("Decompression failed: Expected WAV buffer of %d bytes, but got %d instead!", expected_length,
                      wav_size);
        }
        free(huffman_buffer);
        free(buffer);
        return wav_buffer;
    }
    default:
        LOG_FATAL("Compression Type $%02X not supported!", compression_type);
    }
}

uint32_t mpq_stream_get_size(const struct MpqStream *MpqStream) { return MpqStream->block->size_uncompressed; }

bool mpq_stream_eof(const struct MpqStream *MpqStream) {
    return MpqStream->position >= MpqStream->block->size_uncompressed;
}

void mpq_stream_seek(struct MpqStream *mpq_stream, int64_t position, int32_t origin) {
    switch (origin) {
    case SEEK_SET:
        mpq_stream->position = (uint32_t)position;
        break;
    case SEEK_CUR:
        mpq_stream->position += (uint32_t)position;
        break;
    case SEEK_END:
        mpq_stream->position = mpq_stream->block->size_uncompressed + (uint32_t)position;
        break;
    default:
        LOG_FATAL("Invalid origin for seek: %d", origin);
    }

    if (mpq_stream->position < 0) {
        mpq_stream->position = 0;
    }

    if (mpq_stream->position >= mpq_stream->block->size_uncompressed) {
        mpq_stream->position = mpq_stream->block->size_uncompressed;
    }
}

uint32_t mpq_stream_tell(const struct MpqStream *mpq_stream) { return mpq_stream->position; }
