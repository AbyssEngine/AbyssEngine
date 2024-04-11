#ifndef ABYSS_RING_BUFFER_H
#define ABYSS_RING_BUFFER_H

#include <stdint.h>

struct ring_buffer {
    uint32_t size;
    uint32_t read_position;
    uint32_t write_position;
    uint32_t remaining_to_read;
    uint32_t remaining_to_write;
    char    *buffer;
};

struct ring_buffer *ring_buffer_create(uint32_t size);
void                ring_buffer_free(struct ring_buffer *ring_buffer);
void                ring_buffer_write(struct ring_buffer *ring_buffer, const char *data, uint32_t length);
uint32_t            ring_buffer_read(struct ring_buffer *ring_buffer, char *buffer, uint32_t length);

#endif // ABYSS_RING_BUFFER_H
