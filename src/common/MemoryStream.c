#include "MemoryStream.h"

#include "Logging.h"

#include <string.h>

struct MemoryStream *memory_stream_create(bool preserve_buffer) {
    struct MemoryStream *memory_stream = malloc(sizeof(struct MemoryStream));
    memset(memory_stream, 0, sizeof(struct MemoryStream));

    memory_stream->buffer          = calloc(MEMORY_STREAM_INITIAL_CAPACITY, sizeof(uint8_t));
    memory_stream->preserve_buffer = preserve_buffer;
    memory_stream->read_only       = false;
    memory_stream->capacity        = MEMORY_STREAM_INITIAL_CAPACITY;
    return memory_stream;
}

struct MemoryStream *memory_stream_create_from_existing_buffer(void *buffer, size_t buffer_len) {
    struct MemoryStream *memory_stream = malloc(sizeof(struct MemoryStream));
    memset(memory_stream, 0, sizeof(struct MemoryStream));

    memory_stream->buffer          = (uint8_t *)buffer;
    memory_stream->preserve_buffer = true;
    memory_stream->read_only       = true;
    memory_stream->capacity        = buffer_len;
    memory_stream->write_position  = buffer_len;

    return memory_stream;
}

void memory_stream_free(struct MemoryStream *memory_stream) {
    if (!memory_stream->preserve_buffer) {
        free(memory_stream->buffer);
    }
    free(memory_stream);
}

void memory_stream_resize(struct MemoryStream *memory_stream, size_t new_size) {
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

    memory_stream->buffer = realloc(memory_stream->buffer, memory_stream->capacity);
    if (memory_stream->buffer == NULL) {
        LOG_FATAL("Failed to allocate memory for memory stream!");
    }
}

void memory_stream_write_uint8(struct MemoryStream *memory_stream, uint8_t value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(uint8_t));

    *(uint8_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(uint8_t);
}

void memory_stream_write_int8(struct MemoryStream *memory_stream, int8_t value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(int8_t));

    *(int8_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(int8_t);
}

void memory_stream_write_uint16(struct MemoryStream *memory_stream, uint16_t value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(uint16_t));

    *(uint16_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(uint16_t);
}

void memory_stream_write_int16(struct MemoryStream *memory_stream, int16_t value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(int16_t));

    *(int16_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(int16_t);
}

void memory_stream_write_uint32(struct MemoryStream *memory_stream, uint32_t value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(uint32_t));

    *(uint32_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(uint32_t);
}

void memory_stream_write_int32(struct MemoryStream *memory_stream, int32_t value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(int32_t));

    *(int32_t *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(int32_t);
}

void memory_stream_write_double(struct MemoryStream *memory_stream, double value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(double));

    *(double *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(double);
}

void memory_stream_write_float(struct MemoryStream *memory_stream, float value) {
    memory_stream_resize(memory_stream, memory_stream->write_position + sizeof(float));

    *(float *)&memory_stream->buffer[memory_stream->write_position] = value;

    memory_stream->write_position += sizeof(float);
}

int memory_stream_seek(struct MemoryStream *memory_stream, size_t position) {
    if (position > memory_stream->write_position) {
        memory_stream->read_position = memory_stream->write_position;
        return -1;
    }

    memory_stream->read_position = position;
    return 0;
}

uint8_t memory_stream_read_uint8(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(uint8_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    uint8_t result = *(uint8_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(uint8_t);
    return result;
}

int8_t memory_stream_read_int8(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(int8_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    int8_t result = *(int8_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(int8_t);
    return result;
}

uint16_t memory_stream_read_uint16(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(uint16_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    uint16_t result = *(uint16_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(uint16_t);
    return result;
}

int16_t memory_stream_read_int16(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(int16_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    int16_t result = *(int16_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(int16_t);
    return result;
}

uint32_t memory_stream_read_uint32(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(uint32_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    uint32_t result = *(uint32_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(uint32_t);
    return result;
}

int32_t memory_stream_read_int32(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(int32_t)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    int32_t result = *(int32_t *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(int32_t);
    return result;
}

double memory_stream_read_double(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(double)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    double result = *(double *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(double);
    return result;
}

float memory_stream_read_float(struct MemoryStream *memory_stream) {
    if (memory_stream->write_position - memory_stream->read_position < sizeof(float)) {
        LOG_ERROR("Attempted to read past end of stream!");
    }

    float result = *(float *)&memory_stream->buffer[memory_stream->read_position];

    memory_stream->read_position += sizeof(float);
    return result;
}

void memory_stream_read_skip_bytes(struct MemoryStream *memory_stream, size_t bytes) {
    if (memory_stream->write_position - memory_stream->read_position < bytes) {
        LOG_ERROR("Attempted to read past end of stream!");
    }
    memory_stream->read_position += bytes;
}
size_t memory_stream_get_read_remaining(struct MemoryStream *memory_stream) {
    return memory_stream->write_position - memory_stream->read_position;
}
