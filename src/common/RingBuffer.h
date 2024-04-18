#ifndef ABYSS_RING_BUFFER_H
#define ABYSS_RING_BUFFER_H

#include "../util/Mutex.h"
#include <stdint.h>

typedef struct RingBuffer RingBuffer;

RingBuffer *RingBuffer_Create(uint32_t size);
void        RingBuffer_Free(RingBuffer **ring_buffer);
void        RingBuffer_Write(RingBuffer *ring_buffer, const char *data, uint32_t length);
uint32_t    RingBuffer_Read(RingBuffer *ring_buffer, char *buffer, uint32_t length);
size_t      RingBuffer_GetRemainingToRead(RingBuffer *ring_buffer);

#endif // ABYSS_RING_BUFFER_H
