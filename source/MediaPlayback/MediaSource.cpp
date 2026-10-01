#include "MediaSource.hpp"
#include <string>

RetroFuturaGUI::MediaSource::MediaSource(MediaSource&& other) noexcept
    : _formatContext(other._formatContext)
{
    other._formatContext = nullptr;
}

RetroFuturaGUI::MediaSource& RetroFuturaGUI::MediaSource::operator=(MediaSource&& other) noexcept
{
    if(this == &other)
        return *this;

    Close();

    _formatContext = other._formatContext;
    other._formatContext = nullptr;

    return *this;
}

i32 RetroFuturaGUI::MediaSource::FindBestStream(const AVMediaType type) const
{
    if(!_formatContext)
        return -1;

    const i32 index = av_find_best_stream(_formatContext, type, -1, -1, nullptr, 0);

    // av_find_best_stream reports failure as a negative AVERROR code; callers only need "none"
    return index >= 0 ? index : -1;
}

const AVStream* RetroFuturaGUI::MediaSource::GetStream(const i32 index) const
{
    if(!_formatContext)
        return nullptr;

    if(index < 0 || static_cast<u32>(index) >= _formatContext->nb_streams)
        return nullptr;

    return _formatContext->streams[index];
}

u32 RetroFuturaGUI::MediaSource::GetStreamCount() const
{
    if(!_formatContext)
        return 0;

    return _formatContext->nb_streams;
}

RetroFuturaGUI::MediaSource::~MediaSource()
{
    Close();
}

bool RetroFuturaGUI::MediaSource::Open(std::string_view path)
{
    Close();

    const std::string filepath(path);

    if(avformat_open_input(&_formatContext, filepath.c_str(), nullptr, nullptr) < 0)
        return false;

    if(!_formatContext)
        return false;

    if(avformat_find_stream_info(_formatContext, nullptr) < 0)
    {
        Close();
        return false;
    }

    return true;
}


void RetroFuturaGUI::MediaSource::Close()
{
    if(_formatContext)
        avformat_close_input(&_formatContext);
}

i32 RetroFuturaGUI::MediaSource::ReadPacket(AVPacket* packet)
{
    if(!_formatContext)
        return AVERROR(EINVAL);

    if(!packet)
        return AVERROR(EINVAL);

    return av_read_frame(_formatContext, packet);
}

bool RetroFuturaGUI::MediaSource::IsOpen() const
{ 
    return _formatContext != nullptr; 
}

i32 RetroFuturaGUI::MediaSource::Seek(const i64 milliseconds)
{
    if(!_formatContext)
        return AVERROR(EINVAL);

    if(milliseconds < 0)
        return AVERROR(EINVAL);

    // With stream index -1 FFmpeg reads the timestamp in AV_TIME_BASE units (microseconds)
    i64 timestamp = av_rescale(milliseconds, AV_TIME_BASE, 1000);

    // That timestamp is absolute, and some containers don't start at zero.
    if(_formatContext->start_time != AV_NOPTS_VALUE)
        timestamp += _formatContext->start_time;

    // BACKWARD lands on the keyframe at or before the target instead of the one after it.
    return av_seek_frame(_formatContext, -1, timestamp, AVSEEK_FLAG_BACKWARD);
}