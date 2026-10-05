#include "Resampler.hpp"
#include <algorithm>

RetroFuturaGUI::Resampler::~Resampler()
{
    Close();
}

bool RetroFuturaGUI::Resampler::Open(const AVStream* stream)
{
    if(!stream)
        return false;

    Close();

    const AVCodecParameters* parameters = stream->codecpar;

    if(!parameters)
        return false;

    const i32 channels = parameters->ch_layout.nb_channels;

    if(channels <= 0 || parameters->sample_rate <= 0)
        return false;

    // mono and stereo stay as they are; more channels are downmixed to stereo,
    // because miniaudio would otherwise just drop everything past the first two.
    // maybe a better solution is possible for this
    av_channel_layout_default(&_outLayout, std::min(channels, 2));
    _outRate = parameters->sample_rate;
    return true;
}

void RetroFuturaGUI::Resampler::Close()
{
    swr_free(&_context);
    av_channel_layout_uninit(&_outLayout);
    av_channel_layout_uninit(&_inLayout);
    _outRate = 0;
    _inFormat = AV_SAMPLE_FMT_NONE;
    _inRate = 0;
    _buffer.clear();
}

bool RetroFuturaGUI::Resampler::IsOpen() const
{
    return _outRate > 0;
}

i32 RetroFuturaGUI::Resampler::Convert(const Frame& frame)
{
    const AVFrame* source = frame.Get();

    if(!source)
        return AVERROR(EINVAL);

    if(!IsOpen())
        return AVERROR(EINVAL);

    // check for first frame, or changed format and rebuild
    if(!inputMatches(source) && !configure(source))
        return AVERROR(EINVAL);

    const i32 capacity = reserve(source->nb_samples);

    if(capacity <= 0)
        return capacity;

    uint8_t* out[] = { reinterpret_cast<uint8_t*>(_buffer.data()) };
    return swr_convert(_context, out, capacity, source->extended_data, source->nb_samples);
}

i32 RetroFuturaGUI::Resampler::Flush()
{
    if(!_context)
        return 0;

    const i32 capacity = reserve(0);

    if(capacity <= 0)
        return capacity;

    uint8_t* out[] = { reinterpret_cast<uint8_t*>(_buffer.data()) };
    return swr_convert(_context, out, capacity, nullptr, 0);
}

const float* RetroFuturaGUI::Resampler::GetData() const
{
    return _buffer.data();
}

u32 RetroFuturaGUI::Resampler::GetChannels() const
{
    return static_cast<u32>(_outLayout.nb_channels);
}

u32 RetroFuturaGUI::Resampler::GetSampleRate() const
{
    return static_cast<u32>(_outRate);
}

bool RetroFuturaGUI::Resampler::configure(const AVFrame* frame)
{
    if(!frame)
        return false;

    swr_free(&_context);
    av_channel_layout_uninit(&_inLayout);
    _inFormat = AV_SAMPLE_FMT_NONE;
    _inRate = 0;

    const AVSampleFormat format = static_cast<AVSampleFormat>(frame->format);

    if(swr_alloc_set_opts2(&_context, &_outLayout, AV_SAMPLE_FMT_FLT, _outRate,
        &frame->ch_layout, format, frame->sample_rate, 0, nullptr) < 0)
        return false;

    if(swr_init(_context) < 0 || av_channel_layout_copy(&_inLayout, &frame->ch_layout) < 0)
    {
        swr_free(&_context);
        return false;
    }

    _inFormat = format;
    _inRate = frame->sample_rate;
    return true;
}

bool RetroFuturaGUI::Resampler::inputMatches(const AVFrame* frame) const
{
    if(!frame)
        return false;

    return _context != nullptr
        && frame->format == _inFormat
        && frame->sample_rate == _inRate
        && av_channel_layout_compare(&frame->ch_layout, &_inLayout) == 0;
}

i32 RetroFuturaGUI::Resampler::reserve(const i32 inSamples)
{
    const i32 needed = swr_get_out_samples(_context, inSamples);

    if(needed <= 0)
        return needed;

    const size_t channels = static_cast<size_t>(_outLayout.nb_channels);
    const size_t size = static_cast<size_t>(needed) * channels;

    if(_buffer.size() < size)
        _buffer.resize(size);

    return static_cast<i32>(_buffer.size() / channels);
}