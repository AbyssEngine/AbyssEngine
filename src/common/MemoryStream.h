#ifndef ABYSS_MEMORY_STREAM_H
#define ABYSS_MEMORY_STREAM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MEMORY_STREAM_INITIAL_CAPACITY 1024

struct MemoryStream {
    uint8_t *buffer;
    size_t   write_position;
    size_t   read_position;
    size_t   capacity;
    bool     preserve_buffer;
    bool     read_only;
};

struct MemoryStream *memory_stream_create(bool preserve_buffer);
struct MemoryStream *memory_stream_create_from_existing_buffer(void *buffer, size_t buffer_len);
void                 memory_stream_free(struct MemoryStream *memory_stream);
void                 memory_stream_resize(struct MemoryStream *memory_stream, size_t new_size);
void                 memory_stream_write_uint8(struct MemoryStream *memory_stream, uint8_t value);
void                 memory_stream_write_int8(struct MemoryStream *memory_stream, int8_t value);
void                 memory_stream_write_uint16(struct MemoryStream *memory_stream, uint16_t value);
void                 memory_stream_write_int16(struct MemoryStream *memory_stream, int16_t value);
void                 memory_stream_write_uint32(struct MemoryStream *memory_stream, uint32_t value);
void                 memory_stream_write_int32(struct MemoryStream *memory_stream, int32_t value);
void                 memory_stream_write_double(struct MemoryStream *memory_stream, double value);
void                 memory_stream_write_float(struct MemoryStream *memory_stream, float value);
int                  memory_stream_seek(struct MemoryStream *memory_stream, size_t position);
uint8_t              memory_stream_read_uint8(struct MemoryStream *memory_stream);
int8_t               memory_stream_read_int8(struct MemoryStream *memory_stream);
uint16_t             memory_stream_read_uint16(struct MemoryStream *memory_stream);
int16_t              memory_stream_read_int16(struct MemoryStream *memory_stream);
uint32_t             memory_stream_read_uint32(struct MemoryStream *memory_stream);
int32_t              memory_stream_read_int32(struct MemoryStream *memory_stream);
double               memory_stream_read_double(struct MemoryStream *memory_stream);
float                memory_stream_read_float(struct MemoryStream *memory_stream);
void                 memory_stream_read_skip_bytes(struct MemoryStream *memory_stream, size_t bytes);
size_t               memory_stream_get_read_remaining(struct MemoryStream *memory_stream);

#endif // ABYSS_MEMORY_STREAM_H
