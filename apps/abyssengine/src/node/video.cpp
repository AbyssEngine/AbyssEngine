#include "video.h"
#include "../common/overload.h"
#include "../engine/engine.h"
#include <absl/cleanup/cleanup.h>
#include <absl/strings/ascii.h>
#include <ios>
#include <utility>

namespace AbyssEngine {
namespace {
const int DecodeBufferSize = 1024;
} // namespace

Video::Video(std::string_view name, LibAbyss::InputStream stream, std::optional<LibAbyss::InputStream> separateAudio)
    : Node(name), _stream(std::move(stream)), _ringBuffer(1024 * 4096), _videoCodecContext(), _audioCodecContext(), _avFrame(), _avBuffer(),
      _yPlane(), _uPlane(), _vPlane(), _sourceRect(), _targetRect() {

    _avBuffer = (unsigned char *)av_malloc(DecodeBufferSize); // AVIO is going to free this automagically... because why not?
    memset(_avBuffer, 0, DecodeBufferSize);

    _avioContext =
        avio_alloc_context(_avBuffer, DecodeBufferSize, 0, this, &Video::VideoStreamReadCallback, nullptr, &Video::VideoStreamSeekCallback);

    _avFormatContext = avformat_alloc_context();
    _avFormatContext->pb = _avioContext;
    _avFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;

    int avError;
    if ((avError = avformat_open_input(&_avFormatContext, "", nullptr, nullptr)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to open AV format context: ", AvErrorCodeToString(avError)));

    if ((avError = avformat_find_stream_info(_avFormatContext, nullptr)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to find stream info: ", AvErrorCodeToString(avError)));

    for (auto i = 0; i < _avFormatContext->nb_streams; i++) {
        if (_avFormatContext->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
            continue;

        _videoStreamIdx = i;
        break;
    }

    if (_videoStreamIdx == -1)
        throw std::runtime_error("Could not locate video stream.");

    for (auto i = 0; i < _avFormatContext->nb_streams; i++) {
        if (_avFormatContext->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
            continue;

        _audioStreamIdx = i;
        break;
    }

    _microsPerFrame = (uint64_t)(1000000 / ((float)_avFormatContext->streams[_videoStreamIdx]->r_frame_rate.num) /
                                 (float)_avFormatContext->streams[_videoStreamIdx]->r_frame_rate.den);

    const auto videoCodecPar = _avFormatContext->streams[_videoStreamIdx]->codecpar;
    auto videoDecoder = avcodec_find_decoder(videoCodecPar->codec_id);

    if (videoDecoder == nullptr)
        throw std::runtime_error("Missing video codec.");

    _videoCodecContext = avcodec_alloc_context3(videoDecoder);
    if ((avError = avcodec_parameters_to_context(_videoCodecContext, videoCodecPar)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to apply parameters to video context: ", AvErrorCodeToString(avError)));

    if ((avError = avcodec_open2(_videoCodecContext, videoDecoder, nullptr)) < 0)
        throw std::runtime_error(absl::StrCat("Failed to open video context: ", AvErrorCodeToString(avError)));

    if (_audioStreamIdx >= 0) {
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
        av_opt_set_channel_layout(_resampleContext, "in_channel_layout", (int64_t)_audioCodecContext->channel_layout, 0);
        av_opt_set_channel_layout(_resampleContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(_resampleContext, "in_sample_rate", _audioCodecContext->sample_rate, 0);
        av_opt_set_int(_resampleContext, "out_sample_rate", 44100, 0);
        av_opt_set_sample_fmt(_resampleContext, "in_sample_fmt", _audioCodecContext->sample_fmt, 0);
        av_opt_set_sample_fmt(_resampleContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

        if ((avError = swr_init(_resampleContext)) < 0)
            throw std::runtime_error(absl::StrCat("Failed to initialize sound re-sampler: ", AvErrorCodeToString(avError)));
    }

    const auto ratio = (float)_videoCodecContext->height / (float)_videoCodecContext->width;

    _sourceRect = {.X = 0, .Y = 0, .Width = _videoCodecContext->width, .Height = _videoCodecContext->height};
    _targetRect = {.X = 0, .Y = (600 / 2) - (int)((float)(800 * ratio) / 2), .Width = 800, .Height = (int)(800 * ratio)};

    _videoTexture = Engine::Get()->GetSystemIO().CreateTexture(ITexture::Format::YUV, _videoCodecContext->width, _videoCodecContext->height);

    _swsContext = sws_getContext(_videoCodecContext->width, _videoCodecContext->height, _videoCodecContext->pix_fmt, _videoCodecContext->width,
                                 _videoCodecContext->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);

    size_t yPlaneSize = _videoCodecContext->width * _videoCodecContext->height;
    size_t uvPlaneSize = _videoCodecContext->width * _videoCodecContext->height / 4;
    _yPlane.resize(yPlaneSize);
    _uPlane.resize(uvPlaneSize);
    _vPlane.resize(uvPlaneSize);
    _uvPitch = _videoCodecContext->width / 2;

    _avFrame = av_frame_alloc();
    _videoTimestamp = av_gettime();

    Engine::Get()->GetSystemIO().SetVideo(this);
    if (separateAudio) {
        _separateAudio = std::make_unique<LibAbyss::AudioStream>(*std::move(separateAudio));
        _separateAudio->Play();
    }
}

Video::~Video() {
    Engine::Get()->GetSystemIO().SetVideo(nullptr);

    av_free(_avioContext->buffer);
    avio_context_free(&_avioContext);
    avcodec_free_context(&_videoCodecContext);
    sws_freeContext(_swsContext);
    if (_audioStreamIdx >= 0) {
        avcodec_free_context(&_audioCodecContext);
        swr_free(&_resampleContext);
    }
    av_frame_free(&_avFrame);
    avformat_close_input(&_avFormatContext);
    avformat_free_context(_avFormatContext);
}

void Video::UpdateCallback(uint32_t ticks) {
    _totalTicks += ticks;
    while (_isPlaying) {
        const auto diff = av_gettime() - _videoTimestamp;
        if (diff < _microsPerFrame)
            break;

        _videoTimestamp += _microsPerFrame;
        while (!ProcessFrame()) {
        }
    }

    Node::UpdateCallback(ticks);
}

void Video::RenderCallback(int offsetX, int offsetY) {
    if (_framesReady)
        _videoTexture->Render(_sourceRect, _targetRect);

    Node::RenderCallback(offsetX, offsetY);
}

void Video::MouseEventCallback(const MouseEvent &event) {
    std::visit(Overload{[](const MouseMoveEvent &evt) {},
                        [this](const MouseButtonEvent &evt) {
                            if (!evt.IsPressed || (evt.Button != eMouseButton::Left) || (_totalTicks < 1000))
                                return;

                            _isPlaying = false;

                            Engine::Get()->GetSystemIO().ResetMouseButtonState();
                        }},
               event);

    Node::MouseEventCallback(event);
}

int Video::VideoStreamRead(uint8_t *buffer, int size) {
    if (!_isPlaying)
        return 0;

    _stream.read((char *)buffer, size);
    if (_stream) {
        return (int)_stream.gcount();
    }
    return -1;
}

int64_t Video::VideoStreamSeek(int64_t offset, int whence) {
    if (!_isPlaying)
        return -1;
    _stream.clear();

    std::ios_base::seekdir dir;

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
bool Video::ProcessFrame() {
    if (_avFormatContext == nullptr || !_isPlaying)
        return false;

    AVPacket packet;
    absl::Cleanup cleanup_packet([&] { av_packet_unref(&packet); });
    if (av_read_frame(_avFormatContext, &packet) < 0) {
        _isPlaying = false;
        return true;
    }

    if (packet.stream_index == _videoStreamIdx) {
        int avError;

        if ((avError = avcodec_send_packet(_videoCodecContext, &packet)) < 0)
            throw std::runtime_error(absl::StrCat("Error decoding video packet: ", AvErrorCodeToString(avError)));

        if ((avError = avcodec_receive_frame(_videoCodecContext, _avFrame)) < 0)
            throw std::runtime_error(absl::StrCat("Error decoding video packet: ", AvErrorCodeToString(avError)));

        uint8_t *data[AV_NUM_DATA_POINTERS];
        data[0] = _yPlane.data();
        data[1] = _uPlane.data();
        data[2] = _vPlane.data();

        int lineSize[AV_NUM_DATA_POINTERS];
        lineSize[0] = _videoCodecContext->width;
        lineSize[1] = _uvPitch;
        lineSize[2] = _uvPitch;

        _framesReady = true;

        sws_scale(_swsContext, (const unsigned char *const *)_avFrame->data, _avFrame->linesize, 0, _videoCodecContext->height, data, lineSize);
        _videoTexture->SetYUVData(_yPlane, _videoCodecContext->width, _uPlane, _uvPitch, _vPlane, _uvPitch);

        return true;
    }

    if (packet.stream_index == _audioStreamIdx) {
        int avError;

        if ((avError = avcodec_send_packet(_audioCodecContext, &packet)) < 0)
            throw std::runtime_error(absl::StrCat("Error decoding audio packet: ", AvErrorCodeToString(avError)));

        while (true) {
            if ((avError = avcodec_receive_frame(_audioCodecContext, _avFrame)) < 0) {
                if (avError == AVERROR(EAGAIN) || avError == AVERROR_EOF)
                    break;

                throw std::runtime_error(absl::StrCat("Error decoding audio packet: ", AvErrorCodeToString(avError)));
            }

            int _lineSize;
            auto outSamples = swr_get_out_samples(_resampleContext, _avFrame->nb_samples);
            auto audioOutSize = av_samples_get_buffer_size(&_lineSize, 2, outSamples, AV_SAMPLE_FMT_S16, 0);
            uint8_t *ptr[1] = {_audioOutBuffer};
            auto result = swr_convert(_resampleContext, ptr, audioOutSize, (const uint8_t **)_avFrame->data, _avFrame->nb_samples);
            _ringBuffer.PushData(std::span(_audioOutBuffer, result * 4));
        }

        return false;
    }

    return false;
}
void Video::StopVideo() { _isPlaying = false; }
int16_t Video::GetAudioSample() {
    uint8_t data[2] = {};
    _ringBuffer.ReadData(std::span(data, 2));
    int16_t sample = (int16_t)((uint16_t)(data[0] & 0xFF) | ((uint16_t)data[1] << 8));
    if (_separateAudio) {
        sample += _separateAudio->GetSample();
    }
    return sample;
}

std::string Video::AvErrorCodeToString(int avError) {
    char str[2048] = {};

    av_make_error_string(str, 2048, avError);

    return {str};
}

bool Video::GetIsPlaying() const { return _isPlaying; }

} // namespace AbyssEngine
