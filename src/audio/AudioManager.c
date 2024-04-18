#include "AudioManager.h"
#include "../common/AbyssConfiguration.h"
#include "../common/Logging.h"

typedef struct AudioManager {
    bool                audio_available;
    bool                audio_mute;
    SDL_AudioDeviceID   audio_device_id;
    SDL_AudioSpec       audio_spec;
    enum AVSampleFormat out_sample_format;
    AVChannelLayout     channel_layout;
    struct AudioStream *background_music;
    float               volume[AUDIO_SET_VOLUME_TYPE_MAX];
    float               volume_actual[AUDIO_SET_VOLUME_TYPE_MAX];
} AudioManager;

AudioManager *audio_manager;

void AudioManager_FillBuffer(void *userdata, Uint8 *stream, int len) {
    memset(stream, 0, len);

    if (len & 1) {
        LOG_WARN("Audio buffer length is not even, dropping samples...");
        return;
    }

    AudioManager *manager = (AudioManager *)userdata;

    if (audio_manager->audio_mute) {
        return;
    }

    float volume_bgm    = audio_manager->volume_actual[AUDIO_SET_VOLUME_TYPE_MUSIC];
    float volume_master = audio_manager->volume_actual[AUDIO_SET_VOLUME_TYPE_MASTER];

    for (int i = 0; i < len; i += 2) {
        int32_t sample = 0;

        if (manager->background_music != NULL) {
            sample += (int32_t)((double)AudioStream_GetSample(manager->background_music) * volume_bgm);
        }

        if (sample < -32768) {
            sample = -32768;
        } else if (sample > 32767) {
            sample = 32767;
        }

        *(int16_t *)&stream[i] = (int16_t)((float)sample * volume_master);
    }
}

void AudioManager_InitSingleton(void) {
    LOG_INFO("Initializing audio...");

    audio_manager = malloc(sizeof(AudioManager));
    memset(audio_manager, 0, sizeof(AudioManager));

    audio_manager->audio_mute = false;

    SDL_AudioSpec want = {.freq     = 48000,
                          .format   = AUDIO_S16,
                          .channels = 2,
                          .samples  = 512,
                          .callback = AudioManager_FillBuffer,
                          .userdata = audio_manager};

    LOG_DEBUG("Requested audio spec: %d Hz, %d channels, %d samples", want.freq, want.channels, want.samples);

    if ((audio_manager->audio_device_id =
             SDL_OpenAudioDevice(NULL, 0, &want, &audio_manager->audio_spec,
                                 SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE)) < 2) {
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
    case AUDIO_F32SYS:
        audio_manager->out_sample_format = AV_SAMPLE_FMT_FLT;
        break;
    default:
        LOG_FATAL("Invalid audio spec format: 0x%04X", audio_manager->audio_spec.format);
    }

    AudioManager_SetVolume(AUDIO_SET_VOLUME_TYPE_MASTER, AbyssConfiguration_GetMasterVolume());
    AudioManager_SetVolume(AUDIO_SET_VOLUME_TYPE_MUSIC, AbyssConfiguration_GetMusicVolume());
    AudioManager_SetVolume(AUDIO_SET_VOLUME_TYPE_SFX, AbyssConfiguration_GetSfxVolume());
    AudioManager_SetVolume(AUDIO_SET_VOLUME_TYPE_UI, AbyssConfiguration_GetUiVolume());

    av_channel_layout_default(&audio_manager->channel_layout, audio_manager->audio_spec.channels);
    SDL_PauseAudioDevice(audio_manager->audio_device_id, SDL_FALSE);
}

void AudioManager_DestroySingleton(void) {
    LOG_INFO("Finalizing audio...");

    if (audio_manager->audio_available) {
        SDL_PauseAudioDevice(audio_manager->audio_device_id, SDL_TRUE);
        SDL_CloseAudioDevice(audio_manager->audio_device_id);
    }

    if (audio_manager->background_music != NULL) {
        AudioStream_Destroy(&audio_manager->background_music);
    }

    free(audio_manager);
}

void AudioManager_Update(void) {}

void AudioManager_PlayMusic(const char *path, const bool loop) {
    if (audio_manager->background_music != NULL) {
        AudioStream_Destroy(&audio_manager->background_music);
    }

    audio_manager->background_music = AudioStream_Create(path);
    AudioStream_SetLoop(audio_manager->background_music, loop);
    AudioStream_Play(audio_manager->background_music);
}
void AudioManager_SetVolume(enum AudioSetVolumeType audio_set_volume_type, float volume) {
    audio_manager->volume[audio_set_volume_type]        = volume;
    audio_manager->volume_actual[audio_set_volume_type] = powf(volume, 2.0f);

    if (audio_manager->volume[audio_set_volume_type] > 1.0f) {
        audio_manager->volume[audio_set_volume_type] = 1.0f;
    } else if (audio_manager->volume[audio_set_volume_type] < 0.0f) {
        audio_manager->volume[audio_set_volume_type] = 0.0f;
    }

    if (audio_manager->volume_actual[audio_set_volume_type] > 1.0f) {
        audio_manager->volume_actual[audio_set_volume_type] = 1.0f;
    } else if (audio_manager->volume_actual[audio_set_volume_type] < 0.0f) {
        audio_manager->volume_actual[audio_set_volume_type] = 0.0f;
    }
}

SDL_AudioSpec AudioManager_GetAudioSpec(void) { return audio_manager->audio_spec; }

AVChannelLayout AudioManager_GetChannelLayout(void) { return audio_manager->channel_layout; }

enum AVSampleFormat AudioManager_GetSampleFormat(void) { return audio_manager->out_sample_format; }
