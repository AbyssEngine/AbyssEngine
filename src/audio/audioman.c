#include "audioman.h"
#include "../common/config.h"
#include "../common/log.h"
bool audio_available;
bool audio_mute;

void audioman_fill_buffer(void *userdata, Uint8 *stream, int len) {
    if (audio_mute || config.audio.master_volume == 0) {
        memset(stream, 0, len);
        return;
    }

    memset(stream, 0, len);
}

void audioman_init(void) {
    LOG_INFO("Initializing audio...");

    audio_mute = false;

    SDL_AudioSpec want = {
        .freq     = 44100,
        .format   = AUDIO_S16LSB,
        .channels = 2,
        .samples  = 512,
        .callback = NULL,
    };

    LOG_DEBUG("Requested audio spec: %d Hz, %d channels, %d samples", want.freq, want.channels, want.samples);

    SDL_AudioSpec have;
    if (SDL_OpenAudio(&want, &have) != 0) {
        LOG_ERROR("Failed to open audio: %s", SDL_GetError());
        audio_available = false;
        return;
    }

    LOG_DEBUG("Obtained audio spec: %d Hz, %d channels, %d samples", have.freq, have.channels, have.samples);
}

void audioman_free(void) {
    LOG_INFO("Finalizing audio...");
    SDL_CloseAudio();
}

void audioman_update(void) {}
