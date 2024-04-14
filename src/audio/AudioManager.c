#include "AudioManager.h"
#include "../common/AbyssConfiguration.h"
#include "../common/Logging.h"

struct AudioManager *audio_manager;

void audio_manager_fill_buffer(void *userdata, Uint8 *stream, int len) {
    memset(stream, 0, len);

    if (audio_manager->audio_mute || abyss_configuration.audio.master_volume == 0) {
        return;
    }

    if (audio_manager->background_music != NULL) {
        audio_stream_fill(audio_manager->background_music);
        uint32_t actual_length = (uint32_t)len < audio_manager->background_music->ring_buffer->remaining_to_read
                                     ? len
                                     : audio_manager->background_music->ring_buffer->remaining_to_read;
        ring_buffer_read(audio_manager->background_music->ring_buffer, (char *)stream, actual_length);
    }
}

void audio_manager_init(void) {
    LOG_INFO("Initializing audio...");

    audio_manager = malloc(sizeof(struct AudioManager));

    audio_manager->audio_mute = false;

    SDL_AudioSpec want = {
        .freq     = 44100,
        .format   = AUDIO_S16SYS,
        .channels = 2,
        .samples  = 512,
        .callback = audio_manager_fill_buffer,
    };

    LOG_DEBUG("Requested audio spec: %d Hz, %d channels, %d samples", want.freq, want.channels, want.samples);

    if ((audio_manager->audio_device_id =
             SDL_OpenAudioDevice(NULL, 0, &want, &audio_manager->audio_spec, SDL_AUDIO_ALLOW_ANY_CHANGE)) < 2) {
        LOG_WARN("Failed to open audio: %s", SDL_GetError());
        audio_manager->audio_available = false;
        return;
    }

    LOG_DEBUG("Obtained audio spec: %d Hz, %d channels, %d samples", audio_manager->audio_spec.freq,
              audio_manager->audio_spec.channels, audio_manager->audio_spec.samples);

    SDL_PauseAudioDevice(audio_manager->audio_device_id, SDL_FALSE);
}

void audio_manager_free(void) {
    LOG_INFO("Finalizing audio...");

    if (audio_manager->background_music != NULL) {
        audio_stream_free(audio_manager->background_music);
    }

    if (audio_manager->audio_available) {
        SDL_PauseAudioDevice(audio_manager->audio_device_id, SDL_TRUE);
        SDL_CloseAudioDevice(audio_manager->audio_device_id);
    }
    free(audio_manager);
}

void audio_manager_update(void) {}

void audio_manager_play_bgm(const char *path, bool loop) {
    if (audio_manager->background_music != NULL) {
        audio_stream_free(audio_manager->background_music);
    }

    audio_manager->background_music             = audio_stream_create(path);
    audio_manager->background_music->loop       = loop;
    audio_manager->background_music->is_playing = true;
}
