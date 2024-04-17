#include "AudioStream.h"
#include "../common/FileManager.h"
#include "../common/Logging.h"
#include "AudioManager.h"

#include <stdio.h>

int audio_stream_read_callback(void *opaque, unsigned char *buffer, const int size) {
    struct AudioStream *audio_stream = (struct AudioStream *)opaque;
    int                 result       = (int)mpq_stream_read(audio_stream->stream, buffer, 0, (uint32_t)size);
    return result;
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

    result->ring_buffer = ring_buffer_create(AUDIO_RING_BUFFER_SIZE);
    result->stream      = file_manager_load(path);

    const uint32_t stream_size = mpq_stream_get_size(result->stream);

    size_t decode_buffer_size = stream_size < AUDIO_STREAM_MAX_BUFF_SIZE ? stream_size : AUDIO_STREAM_MAX_BUFF_SIZE;

    result->av_buffer    = av_malloc(decode_buffer_size);
    result->avio_context = avio_alloc_context(result->av_buffer, (int)decode_buffer_size, 0, result,
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

    result->av_resample_context = swr_alloc();
    av_opt_set_int(result->av_resample_context, "in_sample_rate", audio_codec_par->sample_rate, 0);
    av_opt_set_chlayout(result->av_resample_context, "in_chlayout", &result->audio_codec_context->ch_layout, 0);
    av_opt_set_sample_fmt(result->av_resample_context, "in_sample_fmt", audio_codec_par->format, 0);

    av_opt_set_int(result->av_resample_context, "out_sample_rate", audio_manager->audio_spec.freq, 0);
    av_opt_set_chlayout(result->av_resample_context, "out_chlayout", &audio_manager->channel_layout, 0);
    av_opt_set_sample_fmt(result->av_resample_context, "out_sample_fmt", audio_manager->out_sample_format, 0);

    if ((av_error = swr_init(result->av_resample_context)) < 0) {
        LOG_FATAL("Failed to set re-sampler options: %s", av_err2str(av_error));
    }

    if ((av_error = swr_init(result->av_resample_context)) < 0) {
        LOG_FATAL("Failed to initialize re-sampler: %s", av_err2str(av_error));
    }

    result->av_frame = av_frame_alloc();

    return result;
}

void audio_stream_free(struct AudioStream **audio_stream) {
    ring_buffer_free(&(*audio_stream)->ring_buffer);
    av_free((*audio_stream)->avio_context->buffer);
    avio_context_free(&(*audio_stream)->avio_context);

    if ((*audio_stream)->audio_stream_index >= 0) {
        avcodec_free_context(&(*audio_stream)->audio_codec_context);
        swr_free(&(*audio_stream)->av_resample_context);
    }

    av_frame_free(&(*audio_stream)->av_frame);

    avformat_close_input(&(*audio_stream)->av_format_context);
    avformat_free_context((*audio_stream)->av_format_context);

    mpq_stream_free((*audio_stream)->stream);

    free((*audio_stream));
    audio_stream = NULL;
}

void audio_stream_read_frame(struct AudioStream *audio_stream) {
    if (audio_stream->av_format_context == NULL) {
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

    if (avcodec_send_packet(audio_stream->audio_codec_context, packet) < 0) {
        avcodec_flush_buffers(audio_stream->audio_codec_context);
        avformat_flush(audio_stream->av_format_context);
        av_seek_frame(audio_stream->av_format_context, audio_stream->audio_stream_index, 0, AVSEEK_FLAG_FRAME);

        if (!audio_stream->loop) {
            audio_stream->is_playing = false;
            av_packet_free(&packet);
            return;
        }
    }

    while (true) {
        int av_error;
        if ((av_error = avcodec_receive_frame(audio_stream->audio_codec_context, audio_stream->av_frame)) < 0) {
            if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF)
                return;

            LOG_FATAL("Failed to recieve frame: ", av_err2str(av_error));
        }

        int       _lineSize;
        const int out_samples =
            swr_get_out_samples(audio_stream->av_resample_context, audio_stream->av_frame->nb_samples);
        const int audio_out_size = av_samples_get_buffer_size(&_lineSize, 2, out_samples, AV_SAMPLE_FMT_S16, 0);

        uint8_t *audio_out_buffer = malloc(audio_out_size);

        uint8_t  *ptr[1] = {audio_out_buffer};
        const int result =
            swr_convert(audio_stream->av_resample_context, ptr, audio_out_size,
                        (const uint8_t **)audio_stream->av_frame->data, audio_stream->av_frame->nb_samples);
        ring_buffer_write(audio_stream->ring_buffer, (char *)audio_out_buffer, result * 4);
        free(audio_out_buffer);
    }

    //    int av_error;
    //    if ((av_error = avcodec_receive_frame(audio_stream->audio_codec_context, audio_stream->av_frame)) < 0) {
    //        if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF) {
    //            return -1;
    //        }
    //
    //        LOG_FATAL("Failed to receive frame: ", av_err2str(av_error));
    //    }
    //
    //    // const int out_samples = swr_get_out_samples(audio_stream->av_resample_context,
    //    // audio_stream->av_frame->nb_samples);
    //
    //    const int audio_out_size = av_samples_get_buffer_size(NULL, audio_manager->audio_spec.channels,
    //    AV_DECODE_SAMPLES,
    //                                                          audio_manager->out_sample_format, 0);
    //
    //    if (audio_out_size > audio_stream->decode_buffer_size) {
    //        LOG_FATAL("Audio decode failed due to max stream buffer size being too small!");
    //    }
    //
    //    uint8_t *ptr[2] = {audio_stream->av_buffer};
    //
    //    const int result = swr_convert(audio_stream->av_resample_context, ptr, AV_DECODE_SAMPLES,
    //                                   (const uint8_t **)audio_stream->av_frame->data, AV_DECODE_SAMPLES);
    //
    //    if (result < 0) {
    //        LOG_FATAL("Error converting audio.");
    //    }
    //
    //    int bytes_written =
    //        av_get_bytes_per_sample(audio_manager->out_sample_format) * audio_manager->channel_layout.nb_channels *
    //        result;
    //
    //    audio_stream->audio_buffer_written += bytes_written;
    //
    //    av_packet_free(&packet);
    //
    //    return bytes_written;
}

int16_t audio_stream_get_sample(struct AudioStream *audio_stream) {
    if (!audio_stream->is_playing || audio_stream->is_paused) {
        return 0;
    }

    if (audio_stream->ring_buffer->remaining_to_read < 2) {
        audio_stream_read_frame(audio_stream);
    }

    if (audio_stream->ring_buffer->remaining_to_read < 2) {
        return 0;
    }

    uint8_t sample[2];
    ring_buffer_read(audio_stream->ring_buffer, (char *)&sample, sizeof(int8_t) * 2);

    return (int16_t)sample[0] | ((int16_t)sample[1] << 8);
}
