#ifndef ABYSS_MPQ_STREAM_H
#define ABYSS_MPQ_STREAM_H

#include "../types/MPQ.h"
#include <stdint.h>

typedef struct MpqStream MpqStream;

MpqStream *MpqStream_Create(struct MPQ *mpq, const char *file_name);
void       MpqStream_Destroy(MpqStream **mpq_stream);
uint32_t   MpqStream_GetSize(const MpqStream *MpqStream);
uint32_t   MpqStream_Read(MpqStream *mpq_stream, void *buffer, uint32_t offset, uint32_t size);
bool       MpqStream_GetIsEof(const MpqStream *MpqStream);
void       MpqStream_Seek(MpqStream *mpq_stream, int64_t position, int32_t origin);
uint32_t   MpqStream_Tell(const MpqStream *mpq_stream);

#endif // ABYSS_MPQ_STREAM_H
