#include "AudioStream.h"

#include "../common/FileManager.h"
#include "../common/Logging.h"
#include "../managers/AudioManager.h"
#include <libavutil/opt.h>
#include <stdio.h>

struct AudioStream {
    bool                    is_playing;
    bool                    is_paused;
    bool                    loop;
    int                     audio_stream_index;
    struct MpqStream       *stream;
    unsigned char          *av_buffer;
    struct AVIOContext     *avio_context;
    struct AVFormatContext *av_format_context;
    struct AVCodecContext  *audio_codec_context;
    struct SwrContext      *av_resample_context;
    struct AVFrame         *av_frame;
    struct RingBuffer      *ring_buffer;
    struct Mutex           *mutex;
    uint8_t                *audio_out_buffer;
};

void AudioStream__ReadFrame(const struct AudioStream *audio_stream);

#define AUDIO_STREAM_DECODE_BUFFER_SIZE (1024 * 8)
#define AUDIO_STREAM_MAX_BUFF_SIZE      (1024 * 10)
#define AUDIO_RING_BUFFER_SIZE          (1024 * 1024)

int audio_stream_read_callback(void *opaque, unsigned char *buffer, const int size) {
    const AudioStream *audio_stream = (AudioStream *)opaque;

    const int result = MpqStream_Read(audio_stream->stream, buffer, 0, size);
    return result;
}

int64_t audio_stream_seek_callback(void *opaque, const int64_t offset, const int whence) {
    const AudioStream *audio_stream = (AudioStream *)opaque;
    if (whence == AVSEEK_SIZE) {
        return MpqStream_GetSize(audio_stream->stream);
    }

    MpqStream_Seek(audio_stream->stream, offset, whence);
    return MpqStream_Tell(audio_stream->stream);
}

AudioStream *AudioStream_Create(const char *path) {
    AudioStream *result = malloc(sizeof(AudioStream));
    memset(result, 0, sizeof(AudioStream));

    result->ring_buffer      = RingBuffer_Create(AUDIO_RING_BUFFER_SIZE);
    result->stream           = FileManager_OpenFile(path);
    result->mutex            = mutex_create();
    result->audio_out_buffer = malloc(AUDIO_STREAM_DECODE_BUFFER_SIZE);

    const uint32_t stream_size = MpqStream_GetSize(result->stream);

    const size_t decode_buffer_size =
        stream_size < AUDIO_STREAM_MAX_BUFF_SIZE ? stream_size : AUDIO_STREAM_MAX_BUFF_SIZE;

    result->av_buffer    = av_malloc(decode_buffer_size);
    result->avio_context = avio_alloc_context(result->av_buffer, decode_buffer_size, 0, result,
                                              audio_stream_read_callback, NULL, audio_stream_seek_callback);

    result->av_format_context         = avformat_alloc_context();
    result->av_format_context->pb     = result->avio_context;
    result->av_format_context->flags |= AVFMT_FLAG_CUSTOM_IO;

    int av_error;

    if ((av_error = avformat_open_input(&result->av_format_context, "", NULL, NULL)) < 0) {
        LOG_FATAL("Failed to open audio stream: %s", av_err2str(av_error));
    }

    if ((av_error = avformat_find_stream_info(result->av_format_context, NULL)) < 0) {
        LOG_FATAL("Failed to find stream info: %s", av_err2str(av_error));
    }

    result->audio_stream_index = -1;
    for (uint32_t i = 0; i < result->av_format_context->nb_streams; i++) {
        if (result->av_format_context->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }

        result->audio_stream_index = (int)i;
        break;
    }

    if (result->audio_stream_index == -1) {
        LOG_FATAL("No audio stream found for '%s'!", path);
    }

    const struct AVCodecParameters *audio_codec_par =
        result->av_format_context->streams[result->audio_stream_index]->codecpar;

    const struct AVCodec *audio_decoder = avcodec_find_decoder(audio_codec_par->codec_id);

    if (audio_decoder == NULL) {
        LOG_FATAL("Missing audio codec for '%s'!", path);
    }

    LOG_DEBUG("Using %s to decode %s", audio_decoder->long_name, path);

    result->audio_codec_context = avcodec_alloc_context3(audio_decoder);
    if ((av_error = avcodec_parameters_to_context(result->audio_codec_context, audio_codec_par)) < 0) {
        LOG_FATAL("Failed to copy codec parameters to decoder context: %s", av_err2str(av_error));
    }

    if ((av_error = avcodec_open2(result->audio_codec_context, audio_decoder, NULL)) < 0) {
        LOG_FATAL("Failed to open the audio codec: ", av_err2str(av_error));
    }

    AVChannelLayout channel_layout = AudioManager_GetChannelLayout();

    result->av_resample_context = swr_alloc();
    av_opt_set_int(result->av_resample_context, "in_sample_rate", audio_codec_par->sample_rate, 0);
    av_opt_set_chlayout(result->av_resample_context, "in_chlayout", &result->audio_codec_context->ch_layout, 0);
    av_opt_set_sample_fmt(result->av_resample_context, "in_sample_fmt", audio_codec_par->format, 0);

    av_opt_set_int(result->av_resample_context, "out_sample_rate", AudioManager_GetAudioSpec().freq, 0);
    av_opt_set_chlayout(result->av_resample_context, "out_chlayout", &channel_layout, 0);
    av_opt_set_sample_fmt(result->av_resample_context, "out_sample_fmt", AudioManager_GetSampleFormat(), 0);

    if ((av_error = swr_init(result->av_resample_context)) < 0) {
        LOG_FATAL("Failed to set re-sampler options: %s", av_err2str(av_error));
    }

    if ((av_error = swr_init(result->av_resample_context)) < 0) {
        LOG_FATAL("Failed to initialize re-sampler: %s", av_err2str(av_error));
    }

    result->av_frame = av_frame_alloc();

    return result;
}

void AudioStream_Destroy(struct AudioStream **audio_stream) {
    mutex_destroy(&(*audio_stream)->mutex);
    RingBuffer_Free(&(*audio_stream)->ring_buffer);
    av_free((*audio_stream)->avio_context->buffer);
    avio_context_free(&(*audio_stream)->avio_context);

    if ((*audio_stream)->audio_stream_index >= 0) {
        avcodec_free_context(&(*audio_stream)->audio_codec_context);
        swr_free(&(*audio_stream)->av_resample_context);
    }

    av_frame_free(&(*audio_stream)->av_frame);

    avformat_close_input(&(*audio_stream)->av_format_context);
    avformat_free_context((*audio_stream)->av_format_context);

    MpqStream_Destroy(&(*audio_stream)->stream);

    free((*audio_stream)->audio_out_buffer);
    free((*audio_stream));
    *audio_stream = NULL;
}

void AudioStream__ReadFrame(const struct AudioStream *audio_stream) {
    if (audio_stream->av_format_context == NULL) {
        return;
    }

    struct AVPacket *packet = av_packet_alloc();
    int              av_error;

    if ((av_error = av_read_frame(audio_stream->av_format_context, packet)) < 0) {
        LOG_FATAL("Error reading audio frame: ", av_err2str(av_error));
    }

    if (packet->stream_index != audio_stream->audio_stream_index) {
        av_packet_free(&packet);
        return;
    }

    if ((av_error = avcodec_send_packet(audio_stream->audio_codec_context, packet)) < 0) {
        LOG_FATAL("Error decoding packet: ", av_err2str(av_error));
    }

    av_packet_free(&packet);

    while (true) {
        if ((av_error = avcodec_receive_frame(audio_stream->audio_codec_context, audio_stream->av_frame)) < 0) {
            if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF) {
                return;
            }

            LOG_FATAL("Failed to receive frame: ", av_err2str(av_error));
        }

        const size_t sample_size = av_get_bytes_per_sample(AudioManager_GetSampleFormat());
        const int    total_samples =
            AUDIO_STREAM_DECODE_BUFFER_SIZE / (sample_size * AudioManager_GetChannelLayout().nb_channels);

        uint8_t  *ptr[1] = {audio_stream->audio_out_buffer};
        const int result =
            swr_convert(audio_stream->av_resample_context, ptr, total_samples,
                        (const uint8_t **)audio_stream->av_frame->data, audio_stream->av_frame->nb_samples);
        RingBuffer_Write(audio_stream->ring_buffer, (char *)audio_stream->audio_out_buffer, result * 4);
    }
}

int16_t AudioStream_GetSample(struct AudioStream *audio_stream) {
    mutex_lock(audio_stream->mutex);

    if (!audio_stream->is_playing || audio_stream->is_paused) {
        mutex_unlock(audio_stream->mutex);
        return 0;
    }

    if (MpqStream_GetIsEof(audio_stream->stream)) {
        avcodec_flush_buffers(audio_stream->audio_codec_context);
        av_seek_frame(audio_stream->av_format_context, audio_stream->audio_stream_index, 0, AVSEEK_FLAG_FRAME);
        audio_stream->is_playing = audio_stream->loop;

        if (!audio_stream->is_playing) {
            mutex_unlock(audio_stream->mutex);
            return 0;
        }
    }

    if (RingBuffer_GetRemainingToRead(audio_stream->ring_buffer) < 2) {
        AudioStream__ReadFrame(audio_stream);
    }

    if (RingBuffer_GetRemainingToRead(audio_stream->ring_buffer) < 2) {
        mutex_unlock(audio_stream->mutex);
        return 0;
    }

    int16_t sample;
    RingBuffer_Read(audio_stream->ring_buffer, (char *)&sample, sizeof(int16_t));

    mutex_unlock(audio_stream->mutex);

    return sample;
}
bool AudioStream_IsLooping(const AudioStream *audio_stream) { return audio_stream->loop; }

void AudioStream_SetLoop(AudioStream *audio_stream, bool loop) { audio_stream->loop = loop; }

void AudioStream_Play(AudioStream *audio_stream) { audio_stream->is_playing = true; }

bool AudioStream_IsPlaying(const AudioStream *audio_stream) { return audio_stream->is_playing; }

void AudioStream_Stop(AudioStream *audio_stream) { audio_stream->is_playing = false; }
