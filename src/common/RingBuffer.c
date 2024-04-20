#include "RingBuffer.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef NO_LOGGING
#undef FAIL_IF_NULL
#define FAIL_IF_NULL(ptr)                                                                                              \
    if (ptr == NULL) {                                                                                                 \
        LOG_FATAL("Failed to allocate memory.");                                                                       \
    }
#define LOG_FATAL(...) assert(0)
#else
#include "Logging.h"
#endif

struct RingBuffer {
    uint32_t size;
    uint32_t read_position;
    uint32_t write_position;
    uint32_t remaining_to_read;
    uint32_t remaining_to_write;
    char    *buffer;
    Mutex   *mutex;
};

RingBuffer *RingBuffer_Create(uint32_t size) {
    assert(size > 0);

    RingBuffer *result = malloc(sizeof(RingBuffer));

    result->size               = size;
    result->read_position      = 0;
    result->write_position     = 0;
    result->remaining_to_read  = 0;
    result->remaining_to_write = size;
    result->buffer             = malloc(size);
    result->mutex              = Mutex_Create();

    memset(result->buffer, 0, size);

    FAIL_IF_NULL(result->buffer);
    return result;
}

void RingBuffer_Destroy(RingBuffer **ring_buffer) {
    Mutex_Destroy(&(*ring_buffer)->mutex);
    free((*ring_buffer)->buffer);
    free(*ring_buffer);

    *ring_buffer = NULL;
}

void RingBuffer_Write(RingBuffer *ring_buffer, const char *data, uint32_t length) {
    Mutex_Lock(ring_buffer->mutex);

    assert(data != NULL);
    assert(length > 0);

    if (ring_buffer->remaining_to_write < length) {
        LOG_FATAL("Not enough space in ring buffer to write %d bytes", length);
    }

    const uint32_t write_position = ring_buffer->write_position;
    const uint32_t size           = ring_buffer->size;
    const uint32_t remaining      = size - write_position;

    if (remaining >= length) {
        memcpy(ring_buffer->buffer + write_position, data, length);
    } else {
        memcpy(ring_buffer->buffer + write_position, data, remaining);
        memcpy(ring_buffer->buffer, data + remaining, length - remaining);
    }

    ring_buffer->write_position      = (write_position + length) % size;
    ring_buffer->remaining_to_read  += length;
    ring_buffer->remaining_to_write -= length;

    Mutex_Unlock(ring_buffer->mutex);
}

uint32_t RingBuffer_Read(RingBuffer *ring_buffer, char *buffer, uint32_t length) {
    assert(ring_buffer != NULL);
    assert(buffer != NULL);
    assert(length > 0);

    Mutex_Lock(ring_buffer->mutex);
    if (ring_buffer->remaining_to_read < length) {
        LOG_FATAL("Not enough data in ring buffer to read %d bytes", length);
    }

    const uint32_t read_position = ring_buffer->read_position;
    const uint32_t size          = ring_buffer->size;
    const uint32_t remaining     = size - read_position;

    if (remaining >= length) {
        memcpy(buffer, ring_buffer->buffer + read_position, length);
    } else {
        memcpy(buffer, ring_buffer->buffer + read_position, remaining);
        memcpy(buffer + remaining, ring_buffer->buffer, length - remaining);
    }

    ring_buffer->read_position       = (read_position + length) % size;
    ring_buffer->remaining_to_read  -= length;
    ring_buffer->remaining_to_write += length;

    Mutex_Unlock(ring_buffer->mutex);
    return length;
}
size_t RingBuffer_GetRemainingToRead(RingBuffer *ring_buffer) {
    Mutex_Lock(ring_buffer->mutex);
    size_t result = ring_buffer->remaining_to_read;
    Mutex_Unlock(ring_buffer->mutex);
    return result;
}
