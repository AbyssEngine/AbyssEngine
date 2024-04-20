#ifndef ABYSS_VIDEO_MANAGER_H
#define ABYSS_VIDEO_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

void    VideoManager_InitializeSingleton(void);
void    VideoManager_DestroySingleton(void);
void    VideoManager_Update(uint64_t delta);
void    VideoManager_Render(void);
bool    VideoManager_IsPlayingVideo(void);
void    VideoManager_PlayVideo(const char *path);
int16_t VideoManager_GetAudioSample(void);
void    VideoManager_StopVideo(void);

#endif // ABYSS_VIDEO_MANAGER_H
