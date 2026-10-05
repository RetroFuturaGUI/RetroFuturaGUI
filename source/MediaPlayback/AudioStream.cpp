#include "AudioStream.hpp"
#include <algorithm>

RetroFuturaGUI::AudioStream::~AudioStream()
{
    Close();
}

bool RetroFuturaGUI::AudioStream::Open(std::string_view path)
{
    Close();

    if(!_mediaSource.Open(path))
        return false;

    _audioIndex = _mediaSource.FindBestStream(AVMEDIA_TYPE_AUDIO);

    if(-1 == _audioIndex)
    {
        Close();
        return false;
    }

    if(!_audioTrack.Open(_mediaSource.GetStream(_audioIndex)))
    {
        Close();
        return false;
    }

    _duration = readDuration();
    _packetQueue.Reset();
    startThreads();
    return true;
}

void RetroFuturaGUI::AudioStream::Close()
{
    stopThreads();
    _audioTrack.Close();
    _mediaSource.Close();
    _duration = 0;
    _audioIndex = -1;
}

bool RetroFuturaGUI::AudioStream::IsOpen() const
{
    return -1 != _audioIndex;
}

ma_data_source* RetroFuturaGUI::AudioStream::GetDataSource()
{
    return _audioTrack.GetDataSource();
}

i64 RetroFuturaGUI::AudioStream::GetPosition() const
{
    return _audioTrack.GetPosition();
}

i64 RetroFuturaGUI::AudioStream::readDuration() const
{
    const AVFormatContext* formatContext { _mediaSource.GetFormatContext() };
    const AVStream* stream { _mediaSource.GetStream(_audioIndex) };

    if(!formatContext)
        return 0;

    if(!stream)
        return 0;

    // the container's length in AV_TIME_BASE units (microseconds)
    if(formatContext->duration != AV_NOPTS_VALUE)
        return formatContext->duration / 1000;

    // the stream's own lengt, in its time_base
    if(stream->duration != AV_NOPTS_VALUE)
        return av_rescale_q(stream->duration, stream->time_base, AVRational { .num = 1, .den = 1000 });

    return 0;
}

i64 RetroFuturaGUI::AudioStream::GetDuration() const
{
    return _duration;
}

bool RetroFuturaGUI::AudioStream::Seek(const i64 milliseconds)
{
    if(!IsOpen())
        return false;

    // A position past the end would make the clock run on from there; seeking to the end ends playback
    const i64 target { 0 < _duration ? std::clamp<i64>(milliseconds, 0, _duration) : std::max<i64>(0, milliseconds) };

    // Both threads out first: the demuxer, the queue and the decoder are reset below, and nothing
    // may be using them meanwhile. The packets still queued are from before the target anyway.
    stopThreads();
    _packetQueue.Reset();

    // Rare for a local file - a stream that can't seek. Reading carries on where it stopped,
    // minus the packets that were queued.
    if(0 > _mediaSource.Seek(target))
    {
        startThreads();
        return false;
    }

    _audioTrack.SetStartPosition(target);
    startThreads(); // the decode thread's Start flushes the decoder, as every seek requires
    return true;
}

bool RetroFuturaGUI::AudioStream::HasEnded() const
{
    return _audioTrack.HasEnded();
}

void RetroFuturaGUI::AudioStream::startThreads()
{
    _audioTrack.Start();
    _readerThread = std::thread(&AudioStream::readPackets, this);
}

void RetroFuturaGUI::AudioStream::stopThreads()
{
    _audioTrack.Stop(); // aborts the queue, which also lets a reader blocked in PushPacket out

    if(_readerThread.joinable())
        _readerThread.join();
}

void RetroFuturaGUI::AudioStream::readPackets()
{
    while(true)
    {
        Packet packet;

        // AVERROR_EOF at the end of the file - a read error ends the stream the same way
        if(0 > _mediaSource.ReadPacket(packet.Get()))
            break;

        if(packet->stream_index != _audioIndex)
            continue; // video, subtitles - not ours

        // false means the queue was aborted - a Stop or a seek, not the end of the file
        if(!_packetQueue.PushPacket(std::move(packet)))
            return;
    }

    _packetQueue.Finish();
}