#ifndef ABYSS_MEMORY_STREAM_H
#define ABYSS_MEMORY_STREAM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct MemoryStream MemoryStream;

MemoryStream *MemoryStream_Create(bool preserve_buffer);
MemoryStream *MemoryStream_CreateFromExistingBuffer(void *buffer, size_t buffer_len);
void          MemoryStream_Destroy(MemoryStream **memory_stream);
void          MemoryStream_Resize(MemoryStream *memory_stream, size_t new_size);
void          MemoryStream_WriteUint8(MemoryStream *memory_stream, uint8_t value);
void          MemoryStream_WriteInt8(MemoryStream *memory_stream, int8_t value);
void          MemoryStream_WriteUint16(MemoryStream *memory_stream, uint16_t value);
void          MemoryStream_WriteInt16(MemoryStream *memory_stream, int16_t value);
void          MemoryStream_WriteUint32(MemoryStream *memory_stream, uint32_t value);
void          MemoryStream_WriteInt32(MemoryStream *memory_stream, int32_t value);
void          MemoryStream_WriteDouble(MemoryStream *memory_stream, double value);
void          MemoryStream_WriteFloat(MemoryStream *memory_stream, float value);
int           MemoryStream_Seek(MemoryStream *memory_stream, size_t position);
uint8_t       MemoryStream_ReadUint8(MemoryStream *memory_stream);
int8_t        MemoryStream_ReadInt8(MemoryStream *memory_stream);
uint16_t      MemoryStream_ReadUint16(MemoryStream *memory_stream);
int16_t       MemoryStream_ReadInt16(MemoryStream *memory_stream);
uint32_t      MemoryStream_ReadUint32(MemoryStream *memory_stream);
int32_t       MemoryStream_ReadInt32(MemoryStream *memory_stream);
double        MemoryStream_ReadDouble(MemoryStream *memory_stream);
float         MemoryStream_ReadFloat(MemoryStream *memory_stream);
void          MemoryStream_SkipBytes(MemoryStream *memory_stream, size_t bytes);
size_t        MemoryStream_GetBytesAvailableToRead(const MemoryStream *memory_stream);
uint8_t      *MemoryStream_GetBuffer(const MemoryStream *memory_stream);
size_t        MemoryStream_GetTotalBytesWritten(const MemoryStream *memory_stream);

#endif // ABYSS_MEMORY_STREAM_H
