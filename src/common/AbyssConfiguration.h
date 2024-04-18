#ifndef ABYSS_CONFIG_H
#define ABYSS_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void        AbyssConfiguration_LoadSingleton(const char *file_path);
void        AbyssConfiguration_DestroySingleton(void);
void        AbyssConfiguration_AddMpq(const char *mpq_file);
const char *AbyssConfiguration_GetLocale(void);
size_t      AbyssConfiguration_GetMpqCount(void);
const char *AbyssConfiguration_GetMpqFileName(size_t index);
float       AbyssConfiguration_GetInitialScale(void);
const char *AbyssConfiguration_GetScaleQuality(void);
bool        AbyssConfiguration_GetFullScreen(void);
void        AbyssConfiguration_SetFullScreen(bool fullscreen);
float       AbyssConfiguration_GetMasterVolume(void);
float       AbyssConfiguration_GetMusicVolume(void);
float       AbyssConfiguration_GetSfxVolume(void);
float       AbyssConfiguration_GetUiVolume(void);

#endif // ABYSS_CONFIG_H
