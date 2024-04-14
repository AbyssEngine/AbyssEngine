#include "AudioStream.h"
#include "../common/FileManager.h"
#include "../common/Logging.h"
#include "AudioManager.h"

int audio_stream_read_callback(void *opaque, unsigned char *buffer, const int size) {
    struct AudioStream *audio_stream = (struct AudioStream *)opaque;

    return (int)mpq_stream_read(audio_stream->stream, buffer, 0, size);
}

int64_t audio_stream_seek_callback(void *opaque, const int64_t offset, const int whence) {
    struct AudioStream *audio_stream = (struct AudioStream *)opaque;
    if (whence == AVSEEK_SIZE) {
        return mpq_stream_get_size(audio_stream->stream);
    }

    mpq_stream_seek(audio_stream->stream, offset, whence);
    return mpq_stream_tell(audio_stream->stream);
}

struct AudioStream *audio_stream_create(const char *path) {
    struct AudioStream *result = malloc(sizeof(struct AudioStream));
    memset(result, 0, sizeof(struct AudioStream));

    result->stream      = file_manager_load(path);
    result->ring_buffer = ring_buffer_create(AUDIO_STREAM_DECODE_BUFFER_SIZE);

    const uint32_t stream_size = mpq_stream_get_size(result->stream);
    const uint32_t decode_buffer_size =
        stream_size < AUDIO_STREAM_DECODE_BUFFER_SIZE ? stream_size : AUDIO_STREAM_DECODE_BUFFER_SIZE;

    result->av_buffer = av_malloc(decode_buffer_size);
    memset(result->av_buffer, 0, decode_buffer_size);

    result->avio_context = avio_alloc_context(result->av_buffer, (int)decode_buffer_size, 0, result->ring_buffer,
                                              audio_stream_read_callback, NULL, audio_stream_seek_callback);

    result->avio_context->opaque = result;

    result->av_format_context          = avformat_alloc_context();
    result->av_format_context->opaque  = result;
    result->av_format_context->pb      = result->avio_context;
    result->av_format_context->flags  |= AVFMT_FLAG_CUSTOM_IO;

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

    struct AVCodecParameters *audio_codec_par =
        result->av_format_context->streams[result->audio_stream_index]->codecpar;

    const struct AVCodec *audio_decoder = avcodec_find_decoder(audio_codec_par->codec_id);

    if (audio_decoder == NULL) {
        LOG_FATAL("Missing audio codec for '%s'!", path);
    }

    result->av_codec_context = avcodec_alloc_context3(audio_decoder);
    if ((av_error = avcodec_parameters_to_context(result->av_codec_context, audio_codec_par)) < 0) {
        LOG_FATAL("Failed to copy codec parameters to decoder context: %s", av_err2str(av_error));
    }

    if ((av_error = avcodec_open2(result->av_codec_context, audio_decoder, NULL)) < 0) {
        LOG_FATAL("Failed to open the audio codec: ", av_err2str(av_error));
    }

    AVChannelLayout layout_in;
    av_channel_layout_default(&layout_in, audio_codec_par->ch_layout.nb_channels);

    AVChannelLayout layout_out;
    av_channel_layout_default(&layout_out, audio_manager->audio_spec.channels);

    enum AVSampleFormat sample_format;
    switch (audio_manager->audio_spec.format) {
    case AUDIO_U8:
        sample_format = AV_SAMPLE_FMT_U8;
        break;
    case AUDIO_S16SYS:
        sample_format = AV_SAMPLE_FMT_S16;
        break;
    case AUDIO_S32SYS:
        sample_format = AV_SAMPLE_FMT_S32;
        break;
    default:
        LOG_FATAL("Invalid audio spec format: %i", audio_manager->audio_spec.format);
    }

    result->av_resample_context = swr_alloc();

    av_error = swr_alloc_set_opts2(&result->av_resample_context,
                                   &layout_out,                    // output channel layout (e. g. AV_CHANNEL_LAYOUT_*)
                                   sample_format,                  // output sample format (AV_SAMPLE_FMT_*).
                                   audio_manager->audio_spec.freq, // output sample rate (frequency in Hz)
                                   &layout_in,                     // input channel layout (e. g. AV_CHANNEL_LAYOUT_*)
                                   audio_codec_par->format,        // input sample format (AV_SAMPLE_FMT_*).
                                   audio_codec_par->sample_rate,   // input sample rate (frequency in Hz)
                                   0,                              // logging level offset
                                   NULL                            // log_ctx
    );

    if (av_error < 0) {
        LOG_FATAL("Failed to set re-sampler options: %s", av_err2str(av_error));
    }

    if ((av_error = swr_init(result->av_resample_context)) < 0) {
        LOG_FATAL("Failed to initialize re-sampler: %s", av_err2str(av_error));
    }

    result->av_frame = av_frame_alloc();

    return result;
}

void audio_stream_free(struct AudioStream *audio_stream) {
    av_free(audio_stream->avio_context->buffer);
    avio_context_free(&audio_stream->avio_context);

    if (audio_stream->audio_stream_index >= 0) {
        avcodec_free_context(&audio_stream->av_codec_context);
        swr_free(&audio_stream->av_resample_context);
    }

    av_frame_free(&audio_stream->av_frame);

    avformat_close_input(&audio_stream->av_format_context);
    avformat_free_context(audio_stream->av_format_context);

    ring_buffer_free(audio_stream->ring_buffer);
    mpq_stream_free(audio_stream->stream);

    free(audio_stream);
}

void audio_stream_fill(struct AudioStream *audio_stream) {
    if (audio_stream->av_format_context == NULL) {
        return;
    }

    if (!audio_stream->is_playing || audio_stream->is_paused) {
        return;
    }

    if (mpq_stream_eof(audio_stream->stream)) {
        return;
    }

    struct AVPacket *packet = av_packet_alloc();

    if (av_read_frame(audio_stream->av_format_context, packet) < 0) {
        if (audio_stream->loop) {
            av_seek_frame(audio_stream->av_format_context, audio_stream->audio_stream_index, 0, AVSEEK_FLAG_FRAME);
            av_packet_free(&packet);
            return;
        }

        av_seek_frame(audio_stream->av_format_context, audio_stream->audio_stream_index, 0, AVSEEK_FLAG_FRAME);
        audio_stream->is_playing = false;
        av_packet_free(&packet);
        return;
    }

    if (packet->stream_index != audio_stream->audio_stream_index) {
        av_packet_free(&packet);
        return;
    }

    if (avcodec_send_packet(audio_stream->av_codec_context, packet) < 0) {
        avcodec_flush_buffers(audio_stream->av_codec_context);
        avformat_flush(audio_stream->av_format_context);

        if (!audio_stream->loop) {
            audio_stream->is_playing = false;
            av_packet_free(&packet);
            return;
        }
    }

    while (ring_buffer_get_fill_percentage(audio_stream->ring_buffer) < AUDIO_STREAM_WANTED_BUFFER_FILL) {
        int av_error = avcodec_receive_frame(audio_stream->av_codec_context, audio_stream->av_frame);
        if (av_error < 0) {
            if (av_error == AVERROR(EAGAIN)) {
                av_packet_free(&packet);
                return;
            }

            LOG_FATAL("Failed to receive frame: ", av_err2str(av_error));
        }

        int       line_size;
        const int out_samples =
            swr_get_out_samples(audio_stream->av_resample_context, audio_stream->av_frame->nb_samples);
        const int audio_out_size = av_samples_get_buffer_size(&line_size, audio_manager->audio_spec.channels,
                                                              out_samples, AV_SAMPLE_FMT_S16, 0);
        uint8_t  *ptr[1]         = {audio_stream->audio_out_buffer};
        const int result =
            swr_convert(audio_stream->av_resample_context, ptr, audio_out_size,
                        (const uint8_t **)audio_stream->av_frame->data, audio_stream->av_frame->nb_samples);
        ring_buffer_write(audio_stream->ring_buffer, (const char *)audio_stream->audio_out_buffer, result);
    }

    av_packet_free(&packet);
}
