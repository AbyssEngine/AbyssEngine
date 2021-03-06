extern "C" {
#include <libavutil/opt.h>
};

#include "libabyss/streams/audiostream.h"
#include <absl/cleanup/cleanup.h>
#include <absl/strings/str_cat.h>
#include <spdlog/spdlog.h>

namespace LibAbyss {
namespace {
const int DecodeBufferSize = 1024;
} // namespace

AudioStream::AudioStream(InputStream stream) : _stream(std::move(stream)), _ringBuffer(1024 * 1024), _mutex() {
    const auto streamSize = _stream.size();
    const int decodeBufferSize = streamSize < DecodeBufferSize ? streamSize : DecodeBufferSize;

    _avBuffer = (unsigned char *)av_malloc(DecodeBufferSize); // AVIO is going to free this automagically... because why not?
    memset(_avBuffer, 0, DecodeBufferSize);

    _avioContext =
        avio_alloc_context(_avBuffer, decodeBufferSize, 0, this, &AudioStream::StreamReadCallback, nullptr, &AudioStream::StreamSeekCallback);

    _avFormatContext = avformat_alloc_context();
    _avFormatContext->pb = _avioContext;
    _avFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;

    int avError;

    if ((avError = avformat_open_input(&_avFormatContext, "", nullptr, nullptr)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to open AV format context: ", AvErrorCodeToString(avError)));

    if ((avError = avformat_find_stream_info(_avFormatContext, nullptr)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to find stream info: ", AvErrorCodeToString(avError)));

    _audioStreamIdx = -1;
    for (auto i = 0; i < _avFormatContext->nb_streams; i++) {
        if (_avFormatContext->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
            continue;

        _audioStreamIdx = i;
        break;
    }

    if (_audioStreamIdx < 0)
        throw std::runtime_error("No audio stream found");

    const auto audioCodecPar = _avFormatContext->streams[_audioStreamIdx]->codecpar;
    auto audioDecoder = avcodec_find_decoder(audioCodecPar->codec_id);

    if (audioDecoder == nullptr)
        throw std::runtime_error("Missing audio codec.");

    _audioCodecContext = avcodec_alloc_context3(audioDecoder);
    if ((avError = avcodec_parameters_to_context(_audioCodecContext, audioCodecPar)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to apply parameters to audio context: ", AvErrorCodeToString(avError)));

    if ((avError = avcodec_open2(_audioCodecContext, audioDecoder, nullptr)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to open audio context: ", AvErrorCodeToString(avError)));

    _resampleContext = swr_alloc();

    av_opt_set_channel_layout(_resampleContext, "in_channel_layout", av_get_default_channel_layout(_audioCodecContext->channels), 0);
    av_opt_set_int(_resampleContext, "in_sample_rate", _audioCodecContext->sample_rate, 0);
    av_opt_set_sample_fmt(_resampleContext, "in_sample_fmt", _audioCodecContext->sample_fmt, 0);

    av_opt_set_channel_layout(_resampleContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(_resampleContext, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(_resampleContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    if ((avError = swr_init(_resampleContext)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to initialize sound re-sampler: ", AvErrorCodeToString(avError)));

    _avFrame = av_frame_alloc();
}

AudioStream::~AudioStream() {
    av_free(_avioContext->buffer);
    avio_context_free(&_avioContext);
    if (_audioStreamIdx >= 0) {
        avcodec_free_context(&_audioCodecContext);
        swr_free(&_resampleContext);
    }
    av_frame_free(&_avFrame);
    avformat_close_input(&_avFormatContext);
    avformat_free_context(_avFormatContext);
}

int AudioStream::StreamRead(uint8_t *buffer, int size) {
    _stream.read((char *)buffer, size);
    if (_stream) {
        return (int)_stream.gcount();
    }
    return -1;
}

int64_t AudioStream::StreamSeek(int64_t offset, int whence) {
    std::ios_base::seekdir dir;
    _stream.clear();

    switch (whence) {
    case SEEK_SET:
        dir = std::ios_base::beg;
        break;
    case SEEK_CUR:
        dir = std::ios_base::cur;
        break;
    case SEEK_END:
        dir = std::ios_base::end;
        break;
    case AVSEEK_SIZE:
        return _stream.size();
    default:
        return -1;
    }

    _stream.seekg(offset, dir);
    return _stream.tellg();
}

std::string AudioStream::AvErrorCodeToString(int avError) {
    char str[2048] = {};
    av_make_error_string(str, 2048, avError);
    return {str};
}
void AudioStream::Update() {
    if (_avFormatContext == nullptr)
        return;

    int avError;
    AVPacket packet;
    absl::Cleanup cleanup_packet([&] { av_packet_unref(&packet); });

    if ((avError = av_read_frame(_avFormatContext, &packet)) < 0) {
        if (_loop) {
            av_seek_frame(_avFormatContext, _audioStreamIdx, 0, AVSEEK_FLAG_FRAME);
            return;
        } else {
            av_seek_frame(_avFormatContext, _audioStreamIdx, 0, AVSEEK_FLAG_FRAME);
            _isPlaying = false;
            return;
        }
    }

    if (packet.stream_index != _audioStreamIdx)
        return;

    if ((avError = avcodec_send_packet(_audioCodecContext, &packet)) < 0) {
        if (_loop) {
            avcodec_flush_buffers(_audioCodecContext);
            avformat_flush(_avFormatContext);
            av_seek_frame(_avFormatContext, _audioStreamIdx, 0, AVSEEK_FLAG_FRAME);
            return;
        } else {
            avcodec_flush_buffers(_audioCodecContext);
            avformat_flush(_avFormatContext);
            av_seek_frame(_avFormatContext, _audioStreamIdx, 0, AVSEEK_FLAG_FRAME);
            _isPlaying = false;
            return;
        }
    }

    while (true) {
        if ((avError = avcodec_receive_frame(_audioCodecContext, _avFrame)) < 0) {
            if (avError == AVERROR(EAGAIN) || avError == AVERROR_EOF)
                return;

            throw std::runtime_error(absl::StrCat("Error decoding audio packet: ", AvErrorCodeToString(avError)));
        }

        int _lineSize;
        auto outSamples = swr_get_out_samples(_resampleContext, _avFrame->nb_samples);
        auto audioOutSize = av_samples_get_buffer_size(&_lineSize, 2, outSamples, AV_SAMPLE_FMT_S16, 0);
        uint8_t *ptr[1] = {_audioOutBuffer};
        auto result = swr_convert(_resampleContext, ptr, audioOutSize, (const uint8_t **)_avFrame->data, _avFrame->nb_samples);
        _ringBuffer.PushData(std::span(_audioOutBuffer, result * 4));
    }
}
int16_t AudioStream::GetSample() {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_isPaused || (!_isPlaying && _ringBuffer.Available() == 0))
        return 0;

    if (_ringBuffer.Available() == 0 && _isPlaying)
        Update();

    if (!_isPlaying && _ringBuffer.Available() == 0)
        return 0;

    uint8_t data[2];
    _ringBuffer.ReadData(std::span(data, 2));
    return (int16_t)((uint16_t)(data[0] & 0xFF) | ((uint16_t)data[1] << 8));
}

void AudioStream::SetLoop(bool loop) {
    std::lock_guard<std::mutex> lock(_mutex);

    _loop = loop;
}

bool AudioStream::IsLooped() {
    std::lock_guard<std::mutex> lock(_mutex);

    return _loop;
}

bool AudioStream::IsPlaying() {
    std::lock_guard<std::mutex> lock(_mutex);

    return _isPlaying;
}

bool AudioStream::IsPaused() {
    std::lock_guard<std::mutex> lock(_mutex);

    return _isPaused;
}

void AudioStream::Pause() {
    std::lock_guard<std::mutex> lock(_mutex);

    _isPaused = true;
}

void AudioStream::Play() {
    std::lock_guard<std::mutex> lock(_mutex);

    _isPaused = false;
    _isPlaying = true;
}

void AudioStream::Stop() {
    std::lock_guard<std::mutex> lock(_mutex);

    _isPlaying = false;
    _isPaused = false;
}

} // namespace LibAbyss
