#include "AudioManager.h"
#include "../common/AbyssConfiguration.h"
#include "../common/Logging.h"

struct AudioManager *audio_manager;

void audio_manager_fill_buffer(void *userdata, Uint8 *stream, int len) {
    memset(stream, 0, len);

    if (len & 1) {
        LOG_WARN("Audio buffer length is not even, dropping samples...");
        return;
    }

    struct AudioManager *manager = (struct AudioManager *)userdata;

    if (audio_manager->audio_mute || abyss_configuration.audio.master_volume == 0) {
        return;
    }

    for (int i = 0; i < len; i += 2) {
        int32_t sample = 0;

        if (manager->background_music != NULL) {
            sample += (int32_t)((double)audio_stream_get_sample(manager->background_music));
        }

        if (sample < -32768) {
            sample = -32768;
        } else if (sample > 32767) {
            sample = 32767;
        }

        *(int16_t *)&stream[i] = (int16_t)sample;
    }
}

void audio_manager_init(void) {
    LOG_INFO("Initializing audio...");

    audio_manager = malloc(sizeof(struct AudioManager));
    memset(audio_manager, 0, sizeof(struct AudioManager));

    audio_manager->audio_mute = false;

    SDL_AudioSpec want = {.freq     = 44100,
                          .format   = AUDIO_S16,
                          .channels = 2,
                          .samples  = 4096,
                          .callback = audio_manager_fill_buffer,
                          .userdata = audio_manager};

    LOG_DEBUG("Requested audio spec: %d Hz, %d channels, %d samples", want.freq, want.channels, want.samples);

    if ((audio_manager->audio_device_id =
             SDL_OpenAudioDevice(NULL, 0, &want, &audio_manager->audio_spec, SDL_AUDIO_ALLOW_ANY_CHANGE)) < 2) {
        LOG_WARN("Failed to open audio: %s", SDL_GetError());
        audio_manager->audio_available = false;
        return;
    }

    audio_manager->audio_available = true;

    LOG_DEBUG("Obtained audio spec: %d Hz, %d channels, %d samples", audio_manager->audio_spec.freq,
              audio_manager->audio_spec.channels, audio_manager->audio_spec.samples);

    switch (audio_manager->audio_spec.format) {
    case AUDIO_U8:
        audio_manager->out_sample_format = AV_SAMPLE_FMT_U8;
        break;
    case AUDIO_S16SYS:
        audio_manager->out_sample_format = AV_SAMPLE_FMT_S16;
        break;
    case AUDIO_S32SYS:
        audio_manager->out_sample_format = AV_SAMPLE_FMT_S32;
        break;
    default:
        LOG_FATAL("Invalid audio spec format: %i", audio_manager->audio_spec.format);
    }

    av_channel_layout_default(&audio_manager->channel_layout, audio_manager->audio_spec.channels);

    SDL_PauseAudioDevice(audio_manager->audio_device_id, SDL_FALSE);
}

void audio_manager_free(void) {
    LOG_INFO("Finalizing audio...");

    if (audio_manager->audio_available) {
        SDL_PauseAudioDevice(audio_manager->audio_device_id, SDL_TRUE);
        SDL_CloseAudioDevice(audio_manager->audio_device_id);
    }

    if (audio_manager->background_music != NULL) {
        audio_stream_free(&audio_manager->background_music);
    }

    free(audio_manager);
}

void audio_manager_update(void) {}

void audio_manager_play_bgm(const char *path, bool loop) {
    if (audio_manager->background_music != NULL) {
        audio_stream_free(&audio_manager->background_music);
    }

    audio_manager->background_music             = audio_stream_create(path);
    audio_manager->background_music->loop       = loop;
    audio_manager->background_music->is_playing = true;
}
