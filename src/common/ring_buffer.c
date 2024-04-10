#include "ring_buffer.h"
#include "log.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

ring_buffer_t *ring_buffer_create(uint32_t size) {
    ring_buffer_t *result = malloc(sizeof(ring_buffer_t));
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

void ring_buffer_free(ring_buffer_t *ring_buffer) {
    free(ring_buffer->buffer);
    free(ring_buffer);
}

void ring_buffer_write(ring_buffer_t *ring_buffer, const char *data, uint32_t length) {
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
}

uint32_t ring_buffer_read(ring_buffer_t *ring_buffer, char *buffer, uint32_t length) {
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
    return length;
}
