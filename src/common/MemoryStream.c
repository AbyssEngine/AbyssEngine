#include "MemoryStream.h"

#include "Logging.h"
#include <assert.h>
#include <string.h>

#define MEMORY_STREAM_INITIAL_CAPACITY 1024

struct MemoryStream {
    uint8_t *buffer;
    size_t   write_position;
    size_t   read_position;
    size_t   capacity;
    bool     preserve_buffer;
    bool     read_only;
};

MemoryStream *MemoryStream_Create(bool preserve_buffer) {
    MemoryStream *memory_stream = malloc(sizeof(MemoryStream));
    memset(memory_stream, 0, sizeof(MemoryStream));

    memory_stream->buffer          = calloc(MEMORY_STREAM_INITIAL_CAPACITY, sizeof(uint8_t));
    memory_stream->preserve_buffer = preserve_buffer;
    memory_stream->read_only       = false;
    memory_stream->capacity        = MEMORY_STREAM_INITIAL_CAPACITY;
    return memory_stream;
}

MemoryStream *MemoryStream_CreateFromExistingBuffer(void *buffer, const size_t buffer_len) {
    assert(buffer != NULL);
    assert(buffer_len > 0);

    MemoryStream *memory_stream = malloc(sizeof(MemoryStream));
    memset(memory_stream, 0, sizeof(MemoryStream));

    memory_stream->buffer          = (uint8_t *)buffer;
    memory_stream->preserve_buffer = true;
    memory_stream->read_only       = true;
    memory_stream->capacity        = buffer_len;
    memory_stream->write_position  = buffer_len;

    return memory_stream;
}

void MemoryStream_Destroy(MemoryStream **memory_stream) {
    assert(*memory_stream != NULL);

    if (!(*memory_stream)->preserve_buffer) {
        free((*memory_stream)->buffer);
    }
    free(*memory_stream);

    memory_stream = NULL;
}

void MemoryStream_Resize(MemoryStream *memory_stream, const size_t new_size) {
    assert(memory_stream != NULL);
    assert(new_size > 0);

    if (memory_stream->read_only) {
        LOG_FATAL("Attempted to resize a read-only memory stream!");
    }

    if (new_size < memory_stream->write_position) {
        LOG_FATAL("Attempted to make memory stream smaller than it currently is!");
    }

    while (new_size > memory_stream->capacity) {
        if (memory_stream->capacity >= (SIZE_MAX / 2)) {
            // If you get here, something went _terribly_ wrong...
            LOG_FATAL("Attempted to allocate memory greater than the max size of size_t!");
        }

        memory_stream->capacity *= 2;
    }

    memory_stream->buffer =
        realloc(memory_stream->buffer, memory_stream->capacity); // NOLINT(*-suspicious-realloc-usage)
    if (memory_stream->buffer == NULL) {
        LOG_FATAL("Failed to allocate memory for memory stream!");
    }
}

void MemoryStream_WriteUint8(MemoryStream *memory_stream, const uint8_t value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(uint8_t));
    *(uint8_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(uint8_t);
}

void MemoryStream_WriteInt8(MemoryStream *memory_stream, const int8_t value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(int8_t));
    *(int8_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(int8_t);
}

void MemoryStream_WriteUint16(MemoryStream *memory_stream, const uint16_t value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(uint16_t));
    *(uint16_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(uint16_t);
}

void MemoryStream_WriteInt16(MemoryStream *memory_stream, const int16_t value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(int16_t));
    *(int16_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(int16_t);
}

void MemoryStream_WriteUint32(MemoryStream *memory_stream, const uint32_t value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(uint32_t));
    *(uint32_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(uint32_t);
}

void MemoryStream_WriteInt32(MemoryStream *memory_stream, const int32_t value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(int32_t));
    *(int32_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(int32_t);
}

void MemoryStream_WriteDouble(MemoryStream *memory_stream, const double value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(double));
    *(double *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(double);
}

void MemoryStream_WriteFloat(MemoryStream *memory_stream, const float value) {
    assert(memory_stream != NULL);

    MemoryStream_Resize(memory_stream, memory_stream->write_position + sizeof(float));
    *(float *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(float);
}

int MemoryStream_Seek(MemoryStream *memory_stream, const size_t position) {
    assert(memory_stream != NULL);

    if (position > memory_stream->write_position) {
        memory_stream->read_position = memory_stream->write_position;
        return -1;
    }

    memory_stream->read_position = position;
    return 0;
}

uint8_t MemoryStream_ReadUint8(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(uint8_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    uint8_t result = *(uint8_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(uint8_t);
    return result;
}

int8_t MemoryStream_ReadInt8(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(int8_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    int8_t result = *(int8_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(int8_t);
    return result;
}

uint16_t MemoryStream_ReadUint16(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(uint16_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    uint16_t result = *(uint16_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(uint16_t);
    return result;
}

int16_t MemoryStream_ReadInt16(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(int16_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    int16_t result = *(int16_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(int16_t);
    return result;
}

uint32_t MemoryStream_ReadUint32(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(uint32_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    uint32_t result = *(uint32_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(uint32_t);
    return result;
}

int32_t MemoryStream_ReadInt32(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(int32_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    int32_t result = *(int32_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(int32_t);
    return result;
}

double MemoryStream_ReadDouble(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(double)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    double result = *(double *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(double);
    return result;
}

float MemoryStream_ReadFloat(MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < sizeof(float)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    float result = *(float *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(float);
    return result;
}

void MemoryStream_SkipBytes(MemoryStream *memory_stream, const size_t bytes) {
    assert(memory_stream != NULL);

    if (memory_stream->write_position - memory_stream->read_position < bytes) {
        LOG_ERROR("Attempted to read past end of stream!");
    }
    memory_stream->read_position += bytes;
}

size_t MemoryStream_GetBytesAvailableToRead(const MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    return memory_stream->write_position - memory_stream->read_position;
}

uint8_t *MemoryStream_GetBuffer(const MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    return memory_stream->buffer;
}

size_t MemoryStream_GetTotalBytesWritten(const MemoryStream *memory_stream) {
    assert(memory_stream != NULL);

    return memory_stream->write_position;
}
