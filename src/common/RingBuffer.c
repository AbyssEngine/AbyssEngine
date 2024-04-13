#include "RingBuffer.h"
#include "Logging.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct RingBuffer *ring_buffer_create(uint32_t size) {
    struct RingBuffer *result = malloc(sizeof(struct RingBuffer));
    FAIL_IF_NULL(result);
    result->size               = size;
    result->read_position      = 0;
    result->write_position     = 0;
    result->remaining_to_read  = 0;
    result->remaining_to_write = size;
    result->buffer             = malloc(size);
    FAIL_IF_NULL(result->buffer);
    return result;
}

void ring_buffer_free(struct RingBuffer *RingBuffer) {
    free(RingBuffer->buffer);
    free(RingBuffer);
}

void ring_buffer_write(struct RingBuffer *RingBuffer, const char *data, uint32_t length) {
    if (RingBuffer->remaining_to_write < length) {
        LOG_FATAL("Not enough space in ring buffer to write %d bytes", length);
    }

    const uint32_t write_position = RingBuffer->write_position;
    const uint32_t size           = RingBuffer->size;
    const uint32_t remaining      = size - write_position;

    if (remaining >= length) {
        memcpy(RingBuffer->buffer + write_position, data, length);
    } else {
        memcpy(RingBuffer->buffer + write_position, data, remaining);
        memcpy(RingBuffer->buffer, data + remaining, length - remaining);
    }

    RingBuffer->write_position      = (write_position + length) % size;
    RingBuffer->remaining_to_read  += length;
    RingBuffer->remaining_to_write -= length;
}

uint32_t ring_buffer_read(struct RingBuffer *RingBuffer, char *buffer, uint32_t length) {
    if (RingBuffer->remaining_to_read < length) {
        LOG_FATAL("Not enough data in ring buffer to read %d bytes", length);
    }

    const uint32_t read_position = RingBuffer->read_position;
    const uint32_t size          = RingBuffer->size;
    const uint32_t remaining     = size - read_position;

    if (remaining >= length) {
        memcpy(buffer, RingBuffer->buffer + read_position, length);
    } else {
        memcpy(buffer, RingBuffer->buffer + read_position, remaining);
        memcpy(buffer + remaining, RingBuffer->buffer, length - remaining);
    }

    RingBuffer->read_position       = (read_position + length) % size;
    RingBuffer->remaining_to_read  -= length;
    RingBuffer->remaining_to_write += length;
    return length;
}

double ring_buffer_get_fill_percentage(const struct RingBuffer *RingBuffer) {
    return (double)RingBuffer->remaining_to_read / RingBuffer->size;
}
