#ifndef ABYSS_AUDIOMAN_H
#define ABYSS_AUDIOMAN_H

#include "../audio/AudioStream.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

enum AudioSetVolumeType {
    AUDIO_SET_VOLUME_TYPE_MASTER,
    AUDIO_SET_VOLUME_TYPE_MUSIC,
    AUDIO_SET_VOLUME_TYPE_SFX,
    AUDIO_SET_VOLUME_TYPE_UI,
    AUDIO_SET_VOLUME_TYPE_MAX
};

void                AudioManager_InitSingleton(void);
void                AudioManager_DestroySingleton(void);
void                AudioManager_Update(void);
void                AudioManager_PlayMusic(const char *path, bool loop);
void                AudioManager_SetVolume(enum AudioSetVolumeType audio_set_volume_type, float volume);
void                AudioManager_FillBuffer(void *userdata, Uint8 *stream, int len);
SDL_AudioSpec       AudioManager_GetAudioSpec(void);
AVChannelLayout     AudioManager_GetChannelLayout(void);
enum AVSampleFormat AudioManager_GetSampleFormat(void);

#endif // ABYSS_AUDIOMAN_H
