#ifndef ABYSS_AUDIO_STREAM_H
#define ABYSS_AUDIO_STREAM_H

#include "../common/mpq_stream.h"
#include "../common/ring_buffer.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

#define AUDIO_STREAM_DECODE_BUFFER_SIZE 1024

struct audio_stream {
    bool                    is_playing;
    bool                    is_paused;
    bool                    loop;
    int                     audio_stream_index;
    struct mpq_stream      *stream;
    unsigned char          *av_buffer;
    struct AVIOContext     *avio_context;
    struct AVFormatContext *av_format_context;
    struct AVCodecContext  *av_codec_context;
    struct SwrContext      *av_resample_context;
    struct AVFrame         *av_frame;
    struct ring_buffer     *ring_buffer;
};

struct audio_stream *audio_stream_create(const char *path);
void                 audio_stream_free(struct audio_stream *audio_stream);

#endif // ABYSS_AUDIO_STREAM_H
