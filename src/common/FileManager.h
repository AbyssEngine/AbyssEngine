#ifndef ABYSS_FILEMAN_H
#define ABYSS_FILEMAN_H

#include "../types/MPQ.h"
#include "MpqStream.h"

void       FileManager_CreateSingleton(void);
void       FileManager_DestroySingleton(void);
void       FileManager_AddMpq(const char *mpq_path);
MpqStream *FileManager_OpenFile(const char *file_path);

#endif // ABYSS_FILEMAN_H
