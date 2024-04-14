#ifndef ABYSS_AUDIOMAN_H
#define ABYSS_AUDIOMAN_H

#include "AudioStream.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

struct AudioManager {
    bool              audio_available;
    bool              audio_mute;
    SDL_AudioDeviceID audio_device_id;
    SDL_AudioSpec     audio_spec;

    struct AudioStream *background_music;
};

void audio_manager_init(void);
void audio_manager_free(void);
void audio_manager_update(void);
void audio_manager_play_bgm(const char *path, bool loop);

void audio_manager_fill_buffer(void *userdata, Uint8 *stream, int len);

extern struct AudioManager *audio_manager;

#endif // ABYSS_AUDIOMAN_H
