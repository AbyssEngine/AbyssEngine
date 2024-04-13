#ifndef ABYSS_AUDIOMAN_H
#define ABYSS_AUDIOMAN_H

#include <SDL2/SDL.h>
#include <stdbool.h>

void audio_manager_init(void);
void audio_manager_free(void);
void audio_manager_update(void);

void audio_manager_fill_buffer(void *userdata, Uint8 *stream, int len);

extern bool audio_available;
extern bool audio_mute;

#endif // ABYSS_AUDIOMAN_H
