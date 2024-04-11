#include "audio_stream.h"
#include "../common/fileman.h"
#include "../common/log.h"

#define AUDIO_STREAM_DECODE_BUFFER_SIZE 1024

int audio_stream_read_callback(void *opaque, unsigned char *buffer, const int size) {
    struct audio_stream *audio_stream = (struct audio_stream *)opaque;
    return mpq_stream_read(audio_stream->stream, buffer, 0, size);
}

int64_t audio_stream_seek_callback(void *opaque, const int64_t offset, const int whence) {
    struct audio_stream *audio_stream = (struct audio_stream *)opaque;
    if (whence == AVSEEK_SIZE) {
        return mpq_stream_get_size(audio_stream->stream);
    }

    mpq_stream_seek(audio_stream->stream, offset, whence);
    return mpq_stream_tell(audio_stream->stream);
}

struct audio_stream *audio_stream_create(const char *path) {
    struct audio_stream *result = malloc(sizeof(struct audio_stream));
    memset(result, 0, sizeof(struct audio_stream));

    result->stream      = fileman_load(path);
    result->ring_buffer = ring_buffer_create(1024 * 1024);

    const uint32_t stream_size = mpq_stream_get_size(result->stream);
    const uint32_t decode_buffer_size =
        stream_size < AUDIO_STREAM_DECODE_BUFFER_SIZE ? stream_size : AUDIO_STREAM_DECODE_BUFFER_SIZE;

    // NOTE: AVIO is going to somehow free this buffer automatically
    result->av_buffer = av_malloc(AUDIO_STREAM_DECODE_BUFFER_SIZE);
    memset(result->av_buffer, 0, AUDIO_STREAM_DECODE_BUFFER_SIZE);

    result->avio_context =
        avio_alloc_context(result->av_buffer, AUDIO_STREAM_DECODE_BUFFER_SIZE, 0, result->ring_buffer,
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

    return result;
}

void audio_stream_free(struct audio_stream *audio_stream) {
    ring_buffer_free(audio_stream->ring_buffer);
    mpq_stream_free(audio_stream->stream);
    free(audio_stream);
}
