#ifndef ABYSS_RING_BUFFER_H
#define ABYSS_RING_BUFFER_H

#include <stdint.h>

typedef struct {
    uint32_t size;
    uint32_t read_position;
    uint32_t write_position;
    uint32_t remaining_to_read;
    uint32_t remaining_to_write;
    char    *buffer;
} ring_buffer_t;

ring_buffer_t *ring_buffer_create(uint32_t size);
void           ring_buffer_free(ring_buffer_t *ring_buffer);
void           ring_buffer_write(ring_buffer_t *ring_buffer, const char *data, uint32_t length);
uint32_t       ring_buffer_read(ring_buffer_t *ring_buffer, char *buffer, uint32_t length);

#endif // ABYSS_RING_BUFFER_H
