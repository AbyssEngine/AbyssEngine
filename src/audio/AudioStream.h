#ifndef ABYSS_AUDIO_STREAM_H
#define ABYSS_AUDIO_STREAM_H

#include "../common/MpqStream.h"
#include "../common/RingBuffer.h"
#include "../util/Mutex.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

typedef struct AudioStream AudioStream;

AudioStream *AudioStream_Create(const char *path);
void         AudioStream_Destroy(AudioStream **audio_stream);
int16_t      AudioStream_GetSample(AudioStream *audio_stream);
bool         AudioStream_IsLooping(const AudioStream *audio_stream);
void         AudioStream_SetLoop(AudioStream *audio_stream, bool loop);
void         AudioStream_Play(AudioStream *audio_stream);
bool         AudioStream_IsPlaying(const AudioStream *audio_stream);
void         AudioStream_Stop(AudioStream *audio_stream);

#endif // ABYSS_AUDIO_STREAM_H
