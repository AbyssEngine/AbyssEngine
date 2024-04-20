#include "VideoManager.h"

#include "../common/FileManager.h"
#include "../common/Globals.h"
#include "../common/Logging.h"
#include "../common/RingBuffer.h"
#include "../util/Mutex.h"
#include "AudioManager.h"
#include "InputManager.h"
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <stdlib.h>
#include <string.h>

#define AUDIO_RING_BUFFER_SIZE          (1024 * 1024)
#define DECODE_BUFFER_SIZE              (1024 * 8)
#define AUDIO_STREAM_DECODE_BUFFER_SIZE (1024 * 8)
#define AUDIO_STREAM_MAX_BUFF_SIZE      (1024 * 10)
#define AUDIO_RING_BUFFER_SIZE          (1024 * 1024)

typedef struct VideoManager {
    MpqStream         *mpq_stream;
    RingBuffer        *ring_buffer;
    AVFormatContext   *av_format_context;
    AVIOContext       *avio_context;
    SwrContext        *resample_context;
    AVCodecContext    *video_codec_context;
    AVCodecContext    *audio_codec_context;
    AVFrame           *av_frame;
    unsigned char     *av_buffer;
    int                video_stream_index;
    int                audio_stream_index;
    uint64_t           micros_per_frame;
    SDL_Rect           src_rect;
    SDL_Rect           dest_rect;
    SDL_Texture       *texture;
    uint8_t           *y_plane;
    uint8_t           *u_plane;
    uint8_t           *v_plane;
    struct SwsContext *sws_context;
    int                uv_pitch;
    uint64_t           video_timestamp;
    uint64_t           total_ticks;
    bool               is_playing;
    bool               frames_ready;
    uint8_t           *audio_out_buffer;
    Mutex             *mutex;
} VideoManager;

VideoManager *video_manager;

int     VideoManager__StreamRead(void *opaque, uint8_t *buf, int buf_size);
int64_t VideoManager__StreamSeek(void *opaque, int64_t offset, int whence);
bool    VideoManager__ProcessFrame(void);

void VideoManager_InitializeSingleton(void) {
    video_manager = malloc(sizeof(VideoManager));
    memset(video_manager, 0, sizeof(VideoManager));
    video_manager->mutex = Mutex_Create();
}

void VideoManager_DestroySingleton(void) {
    VideoManager_StopVideo();
    Mutex_Destroy(&video_manager->mutex);
    free(video_manager);
}

bool VideoManager_IsPlayingVideo(void) {
    Mutex_Lock(video_manager->mutex);
    bool result = video_manager->is_playing;
    Mutex_Unlock(video_manager->mutex);

    return result;
}

void VideoManager_Update(uint64_t delta) {
    Mutex_Lock(video_manager->mutex);

    bool mouse_button_left;
    InputManager_GetMouseButtons(&mouse_button_left, NULL, NULL);
    if (mouse_button_left) {
        Mutex_Unlock(video_manager->mutex);
        VideoManager_StopVideo();
        InputManager_ResetMouseButtons();
        return;
    }

    video_manager->total_ticks += delta;

    while (video_manager->is_playing) {
        const uint64_t diff = av_gettime() - video_manager->video_timestamp;
        if (diff < video_manager->micros_per_frame) {
            break;
        }

        video_manager->video_timestamp += video_manager->micros_per_frame;
        while (VideoManager__ProcessFrame()) {
        }
    }
    Mutex_Unlock(video_manager->mutex);
}

void VideoManager_Render(void) {
    Mutex_Lock(video_manager->mutex);
    if (!video_manager->frames_ready) {
        Mutex_Unlock(video_manager->mutex);
        return;
    }
    SDL_RenderCopy(sdl_renderer, video_manager->texture, &video_manager->src_rect, &video_manager->dest_rect);
    Mutex_Unlock(video_manager->mutex);
}

void VideoManager_PlayVideo(const char *path) {
    assert(video_manager != NULL);
    Mutex_Lock(video_manager->mutex);

    if (video_manager->is_playing) {
        VideoManager_StopVideo();
    }

    if ((video_manager->mpq_stream = FileManager_OpenFile(path)) == NULL) {
        LOG_FATAL("Failed to load video file '%s'!", path);
    }

    video_manager->is_playing       = true;
    video_manager->frames_ready     = false;
    video_manager->ring_buffer      = RingBuffer_Create(AUDIO_RING_BUFFER_SIZE);
    video_manager->audio_out_buffer = malloc(AUDIO_STREAM_DECODE_BUFFER_SIZE);
    video_manager->av_buffer        = av_malloc(DECODE_BUFFER_SIZE);
    video_manager->avio_context     = avio_alloc_context(video_manager->av_buffer, DECODE_BUFFER_SIZE, 0, video_manager,
                                                         VideoManager__StreamRead, NULL, VideoManager__StreamSeek);

    video_manager->av_format_context        = avformat_alloc_context();
    video_manager->av_format_context->pb    = video_manager->avio_context;
    video_manager->av_format_context->flags = AVFMT_FLAG_CUSTOM_IO;

    int av_error;
    if ((av_error = avformat_open_input(&video_manager->av_format_context, "", NULL, NULL)) < 0) {
        LOG_FATAL("Failed to open AV format context: %s", av_err2str(av_error));
    }

    if ((av_error = avformat_find_stream_info(video_manager->av_format_context, NULL)) < 0) {
        LOG_FATAL("Failed to find video stream info: %s", av_err2str(av_error));
    }

    video_manager->video_stream_index = -1;
    for (uint32_t i = 0; i < video_manager->av_format_context->nb_streams; i++) {
        if (video_manager->av_format_context->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
            continue;
        }

        video_manager->video_stream_index = (int)i;
        break;
    }

    if (video_manager->video_stream_index == -1) {
        LOG_FATAL("Unable to find video stream index.");
    }

    video_manager->audio_stream_index = -1;
    for (uint32_t i = 0; i < video_manager->av_format_context->nb_streams; i++) {
        if (video_manager->av_format_context->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }

        video_manager->audio_stream_index = (int)i;
        break;
    }

    if (video_manager->audio_stream_index == -1) {
        LOG_FATAL("Unable to find audio stream index.");
    }

    video_manager->micros_per_frame =
        (uint64_t)(1000000.0f / ((float)video_manager->av_format_context->streams[video_manager->video_stream_index]
                                     ->r_frame_rate.num /
                                 (float)video_manager->av_format_context->streams[video_manager->video_stream_index]
                                     ->r_frame_rate.den));

    const AVCodecParameters *video_codec_parameters =
        video_manager->av_format_context->streams[video_manager->video_stream_index]->codecpar;
    const AVCodec *video_decoder = avcodec_find_decoder(video_codec_parameters->codec_id);

    if (video_decoder == NULL) {
        LOG_FATAL("Unable to find video decoder.");
    }

    video_manager->video_codec_context = avcodec_alloc_context3(video_decoder);
    if ((av_error = avcodec_parameters_to_context(video_manager->video_codec_context, video_codec_parameters)) < 0) {
        LOG_FATAL("Failed to apply parameters to video context: %s", av_err2str(av_error));
    }

    if ((av_error = avcodec_open2(video_manager->video_codec_context, video_decoder, NULL)) < 0) {
        LOG_FATAL("Failed to open video context: %s", av_err2str(av_error));
    }

    const AVCodecParameters *audio_codec_parameters =
        video_manager->av_format_context->streams[video_manager->audio_stream_index]->codecpar;
    const AVCodec *audio_decoder = avcodec_find_decoder(audio_codec_parameters->codec_id);

    if (audio_decoder == NULL) {
        LOG_FATAL("Unable to find audio decoder.");
    }

    video_manager->audio_codec_context = avcodec_alloc_context3(audio_decoder);
    if ((av_error = avcodec_parameters_to_context(video_manager->audio_codec_context, audio_codec_parameters)) < 0) {
        LOG_FATAL("Failed to apply parameters to audio context: %s", av_err2str(av_error));
    }

    if ((av_error = avcodec_open2(video_manager->audio_codec_context, audio_decoder, NULL)) < 0) {
        LOG_FATAL("Failed to open audio context: %s", av_err2str(av_error));
    }

    AVChannelLayout channel_layout = AudioManager_GetChannelLayout();

    video_manager->resample_context = swr_alloc();
    av_opt_set_int(video_manager->resample_context, "in_sample_rate", video_manager->audio_codec_context->sample_rate,
                   0);
    av_opt_set_chlayout(video_manager->resample_context, "in_chlayout", &video_manager->audio_codec_context->ch_layout,
                        0);
    av_opt_set_sample_fmt(video_manager->resample_context, "in_sample_fmt",
                          video_manager->audio_codec_context->sample_fmt, 0);
    av_opt_set_int(video_manager->resample_context, "out_sample_rate", AudioManager_GetAudioSpec().freq, 0);
    av_opt_set_chlayout(video_manager->resample_context, "out_chlayout", &channel_layout, 0);
    av_opt_set_sample_fmt(video_manager->resample_context, "out_sample_fmt", AudioManager_GetSampleFormat(), 0);

    if ((av_error = swr_init(video_manager->resample_context)) < 0) {
        LOG_FATAL("Failed to initialize resample context: %s", av_err2str(av_error));
    }

    const float ratio =
        (float)video_manager->video_codec_context->height / (float)video_manager->video_codec_context->width;

    video_manager->src_rect.x = 0;
    video_manager->src_rect.y = 0;
    video_manager->src_rect.w = video_manager->video_codec_context->width;
    video_manager->src_rect.h = video_manager->video_codec_context->height;

    video_manager->dest_rect.x = 0;
    video_manager->dest_rect.y = (600 / 2) - (int)(800 * ratio / 2);
    video_manager->dest_rect.w = 800;
    video_manager->dest_rect.h = (int)(800 * ratio);

    video_manager->texture =
        SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                          video_manager->video_codec_context->width, video_manager->video_codec_context->height);

    video_manager->sws_context =
        sws_getContext(video_manager->video_codec_context->width, video_manager->video_codec_context->height,
                       video_manager->video_codec_context->pix_fmt, video_manager->video_codec_context->width,
                       video_manager->video_codec_context->height, AV_PIX_FMT_YUV420P, SWS_POINT, NULL, NULL, NULL);

    const size_t y_plane_size = video_manager->video_codec_context->width * video_manager->video_codec_context->height;
    const size_t uv_plane_size =
        video_manager->video_codec_context->width * video_manager->video_codec_context->height / 4;

    video_manager->y_plane  = malloc(y_plane_size);
    video_manager->u_plane  = malloc(uv_plane_size);
    video_manager->v_plane  = malloc(uv_plane_size);
    video_manager->uv_pitch = video_manager->video_codec_context->width / 2;

    video_manager->av_frame        = av_frame_alloc();
    video_manager->video_timestamp = av_gettime();

    Mutex_Unlock(video_manager->mutex);
}

int16_t VideoManager_GetAudioSample(void) {
    Mutex_Lock(video_manager->mutex);
    if (RingBuffer_GetRemainingToRead(video_manager->ring_buffer) < 2) {
        Mutex_Unlock(video_manager->mutex);
        return 0;
    }
    int16_t sample;
    RingBuffer_Read(video_manager->ring_buffer, (char *)&sample, sizeof(int16_t));

    Mutex_Unlock(video_manager->mutex);
    return sample;
}
void VideoManager_StopVideo(void) {
    Mutex_Lock(video_manager->mutex);

    if (!video_manager->is_playing) {
        Mutex_Unlock(video_manager->mutex);
        return;
    }
    video_manager->is_playing = false;

    av_free(video_manager->avio_context->buffer);
    avio_context_free(&video_manager->avio_context);
    avcodec_free_context(&video_manager->video_codec_context);
    sws_freeContext(video_manager->sws_context);
    avcodec_free_context(&video_manager->audio_codec_context);
    swr_free(&video_manager->resample_context);
    av_frame_free(&video_manager->av_frame);
    avformat_close_input(&video_manager->av_format_context);
    avformat_free_context(video_manager->av_format_context);

    SDL_DestroyTexture(video_manager->texture);
    video_manager->texture = NULL;
    MpqStream_Destroy(&video_manager->mpq_stream);

    free(video_manager->y_plane);
    free(video_manager->u_plane);
    free(video_manager->v_plane);
    free(video_manager->audio_out_buffer);

    RingBuffer_Destroy(&video_manager->ring_buffer);

    Mutex_Unlock(video_manager->mutex);
}

int VideoManager__StreamRead(void *opaque, uint8_t *buf, int buf_size) {
    VideoManager *vm = (VideoManager *)opaque;

    return vm->is_playing ? (int)MpqStream_Read(vm->mpq_stream, buf, 0, buf_size) : 0;
}

int64_t VideoManager__StreamSeek(void *opaque, int64_t offset, int whence) {
    VideoManager *vm = (VideoManager *)opaque;

    if (!vm->is_playing) {
        return -1;
    }

    if (whence == AVSEEK_SIZE) {
        int64_t result = (int)MpqStream_GetSize(vm->mpq_stream);
        return result;
    }

    int64_t result = MpqStream_Tell(vm->mpq_stream);
    return result;
}

bool VideoManager__ProcessFrame(void) {
    if (video_manager->av_format_context == NULL || !video_manager->is_playing) {
        return false;
    }

    AVPacket *packet = av_packet_alloc();

    if (av_read_frame(video_manager->av_format_context, packet) < 0) {
        video_manager->is_playing = false;
        av_packet_free(&packet);

        return true;
    }

    if (packet->stream_index == video_manager->video_stream_index) {
        int av_error;

        if ((av_error = avcodec_send_packet(video_manager->video_codec_context, packet)) < 0) {
            LOG_FATAL("Error sending video packet: %s", av_err2str(av_error));
        }

        if ((av_error = avcodec_receive_frame(video_manager->video_codec_context, video_manager->av_frame)) < 0) {
            LOG_FATAL("Error receiving video packet: %s", av_err2str(av_error));
        }

        uint8_t *data[AV_NUM_DATA_POINTERS];
        memset(data, 0, sizeof(uint8_t *) * AV_NUM_DATA_POINTERS);
        data[0] = video_manager->y_plane;
        data[1] = video_manager->u_plane;
        data[2] = video_manager->v_plane;

        int line_size[AV_NUM_DATA_POINTERS];
        memset(line_size, 0, sizeof(int) * AV_NUM_DATA_POINTERS);
        line_size[0] = video_manager->video_codec_context->width;
        line_size[1] = video_manager->uv_pitch;
        line_size[2] = video_manager->uv_pitch;

        video_manager->frames_ready = true;

        sws_scale(video_manager->sws_context, (const uint8_t *const *)video_manager->av_frame->data,
                  video_manager->av_frame->linesize, 0, video_manager->video_codec_context->height, data, line_size);

        if (SDL_UpdateYUVTexture(video_manager->texture, NULL, video_manager->y_plane,
                                 video_manager->video_codec_context->width, video_manager->u_plane,
                                 video_manager->uv_pitch, video_manager->v_plane, video_manager->uv_pitch) < 0) {
            LOG_FATAL("Cannot set YUV data");
        }
        av_packet_free(&packet);

        return true;
    }

    if (packet->stream_index == video_manager->audio_stream_index) {
        int av_error;

        if ((av_error = avcodec_send_packet(video_manager->audio_codec_context, packet))) {
            LOG_FATAL("Error sending audio packet: %s", av_err2str(av_error));
        }

        while (true) {
            if ((av_error = avcodec_receive_frame(video_manager->audio_codec_context, video_manager->av_frame)) < 0) {
                if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF) {
                    break;
                }
                LOG_FATAL("Error receiving audio packet: %s", av_err2str(av_error));
            }

            const size_t sample_size = av_get_bytes_per_sample(AudioManager_GetSampleFormat());
            const int    total_samples =
                AUDIO_STREAM_DECODE_BUFFER_SIZE / (sample_size * AudioManager_GetChannelLayout().nb_channels);

            uint8_t  *ptr[1] = {video_manager->audio_out_buffer};
            const int result =
                swr_convert(video_manager->resample_context, ptr, total_samples,
                            (const uint8_t **)video_manager->av_frame->data, video_manager->av_frame->nb_samples);

            RingBuffer_Write(video_manager->ring_buffer, (char *)video_manager->audio_out_buffer, result * 4);
        }

        av_packet_free(&packet);
        return false;
    }

    av_packet_free(&packet);
    return false;
}
