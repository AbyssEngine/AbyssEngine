#ifndef ABYSS_MPQ_H
#define ABYSS_MPQ_H

#include "MPQBlock.h"
#include "MPQHash.h"
#include "MPQHeader.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct MPQ MPQ;

MPQ             *MPQ_Load(const char *mpq_path);
void             MPQ_Destroy(MPQ *mpq);
bool             MPQ_FileExists(const MPQ *mpq, const char *file_path);
struct MPQHash  *MPQ_GetFileHash(const MPQ *mpq, const char *file_path);
struct MPQBlock *MPQ_GetBlock(const MPQ *mpq, uint32_t block_index);
uint64_t         MPQ_GetBlockSize(const MPQ *mpq);
FILE            *MPQ_AcquireFileHandle(MPQ *mpq);
void             MPQ_ReleaseFileHandle(MPQ *mpq);

#endif // ABYSS_MPQ_H
