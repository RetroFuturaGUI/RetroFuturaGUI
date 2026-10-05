#include "Decoder.hpp"

RetroFuturaGUI::Decoder::Decoder(Decoder&& other) noexcept
    : _codecContext(other._codecContext)
    , _timeBase(other._timeBase)
    , _streamIndex(other._streamIndex)
{
    other._codecContext = nullptr;
    other._timeBase = { .num = 0, .den = 1 };
    other._streamIndex = -1;
}

RetroFuturaGUI::Decoder& RetroFuturaGUI::Decoder::operator=(Decoder&& other) noexcept
{
    if(this == &other)
        return *this;

    Close();

    _codecContext = other._codecContext;
    _timeBase = other._timeBase;
    _streamIndex = other._streamIndex;

    other._codecContext = nullptr;
    other._timeBase = { .num = 0, .den = 1 };
    other._streamIndex = -1;

    return *this;
}

RetroFuturaGUI::Decoder::~Decoder()
{
    Close();
}

bool RetroFuturaGUI::Decoder::Open(const AVStream* stream)
{
    if(!stream)
        return false;

    Close();

    const AVCodecParameters* parameters = stream->codecpar;

    if(!parameters)
        return false;

    if(!IsCodecFree(parameters->codec_id))
        return false;

    const AVCodec* codec = avcodec_find_decoder(parameters->codec_id);

    if(!codec)
        return false;

    _codecContext = avcodec_alloc_context3(codec);

    if(!_codecContext)
        return false;

    if(avcodec_parameters_to_context(_codecContext, parameters) < 0)
    {
        Close();
        return false;
    }

    if(parameters->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        _codecContext->thread_count = 0;
        _codecContext->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
    }

    if(avcodec_open2(_codecContext, codec, nullptr) < 0)
    {
        Close();
        return false;
    }

    _timeBase = stream->time_base;
    _streamIndex = stream->index;
    return true;
}

void RetroFuturaGUI::Decoder::Close()
{
    if(_codecContext)
        avcodec_free_context(&_codecContext);

    _timeBase = { .num = 0, .den = 1 };
    _streamIndex = -1;
}

i32 RetroFuturaGUI::Decoder::SendPacket(const AVPacket* packet)
{
    if(!_codecContext)
        return AVERROR(EINVAL);

    return avcodec_send_packet(_codecContext, packet);
}

i32 RetroFuturaGUI::Decoder::ReceiveFrame(AVFrame* frame)
{
    if(!_codecContext)
        return AVERROR(EINVAL);

    if(!frame)
        return AVERROR(EINVAL);

    return avcodec_receive_frame(_codecContext, frame);
}

void RetroFuturaGUI::Decoder::Flush()
{
    if(!_codecContext)
        return;

    avcodec_flush_buffers(_codecContext);
}
