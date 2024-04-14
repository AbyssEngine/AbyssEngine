#ifndef ABYSS_AUDIO_STREAM_H
#define ABYSS_AUDIO_STREAM_H

#include "../common/MpqStream.h"
#include "../common/RingBuffer.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

#define AUDIO_STREAM_DECODE_BUFFER_SIZE (1024 * 32)
#define AUDIO_STREAM_WANTED_BUFFER_FILL 0.75

struct AudioStream {
    bool                    is_playing;
    bool                    is_paused;
    bool                    loop;
    int                     audio_stream_index;
    struct MpqStream       *stream;
    unsigned char          *av_buffer;
    struct AVIOContext     *avio_context;
    struct AVFormatContext *av_format_context;
    struct AVCodecContext  *av_codec_context;
    struct SwrContext      *av_resample_context;
    struct AVFrame         *av_frame;
    struct RingBuffer      *ring_buffer;
    uint8_t                 audio_out_buffer[1024 * 16];
};

struct AudioStream *audio_stream_create(const char *path);
void                audio_stream_free(struct AudioStream *audio_stream);
void                audio_stream_fill(struct AudioStream *audio_stream);

#endif // ABYSS_AUDIO_STREAM_H
