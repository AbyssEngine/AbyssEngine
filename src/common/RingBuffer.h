#ifndef ABYSS_RING_BUFFER_H
#define ABYSS_RING_BUFFER_H

#include <stdint.h>

struct RingBuffer {
    uint32_t size;
    uint32_t read_position;
    uint32_t write_position;
    uint32_t remaining_to_read;
    uint32_t remaining_to_write;
    char    *buffer;
};

struct RingBuffer *ring_buffer_create(uint32_t size);
void               ring_buffer_free(struct RingBuffer *RingBuffer);
void               ring_buffer_write(struct RingBuffer *RingBuffer, const char *data, uint32_t length);
uint32_t           ring_buffer_read(struct RingBuffer *RingBuffer, char *buffer, uint32_t length);
double             ring_buffer_get_fill_percentage(const struct RingBuffer *RingBuffer);

#endif // ABYSS_RING_BUFFER_H
