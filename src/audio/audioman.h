#ifndef ABYSS_AUDIOMAN_H
#define ABYSS_AUDIOMAN_H

#include <SDL2/SDL.h>
#include <stdbool.h>

void audioman_init(void);
void audioman_free(void);
void audioman_update(void);

void audioman_fill_buffer(void *userdata, Uint8 *stream, int len);

extern bool audio_available;
extern bool audio_mute;

#endif // ABYSS_AUDIOMAN_H
