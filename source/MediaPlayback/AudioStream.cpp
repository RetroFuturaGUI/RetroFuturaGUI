#include "AudioStream.hpp"
#include "AudioMetadataReader.hpp"
#include "Decoder.hpp"
#include "Packet.hpp"
#include <algorithm>
#include <optional>
#include <span>

RetroFuturaGUI::AudioStream::~AudioStream()
{
    Close();
}

bool RetroFuturaGUI::AudioStream::Open(std::string_view path)
{
    return Open(path, HcaKey {});
}

bool RetroFuturaGUI::AudioStream::Open(std::string_view path, const HcaKey& key)
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

    //An HCA file's cipher from its header; the first block shows whether the key fits
    const AVStream* stream { _mediaSource.GetStream(_audioIndex) };

    if(stream && stream->codecpar && AV_CODEC_ID_HCA == stream->codecpar->codec_id)
    {
        const std::optional<HcaMetadata> header { AudioMetadataReader::ReadHcaHeader(std::span<const u8>(stream->codecpar->extradata, static_cast<uSize>(std::max(stream->codecpar->extradata_size, 0)))) };

        if(!header || !_cipher.Init(header->_CipherType, key) || !decodesFirstBlock(path))
        {
            Close();
            return false;
        }
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
    _cipher.Init(0, HcaKey {});
    _isLooping = false;
    _loopEndPts = INT64_MAX;
    _loopSeekMilliseconds = 0;
    _audioTrack.SetLoop(false, 0, 0);
    _duration = 0;
    _audioIndex = -1;
}

bool RetroFuturaGUI::AudioStream::decodesFirstBlock(std::string_view path) const
{
    MediaSource source {};

    if(!source.Open(path))
        return false;

    const i32 index { source.FindBestStream(AVMEDIA_TYPE_AUDIO) };
    Decoder decoder {};

    if(!decoder.Open(source.GetStream(index)))
        return false;

    Packet packet;

    while(0 <= source.ReadPacket(packet.Get()))
    {
        if(index != packet->stream_index)
        {
            av_packet_unref(packet.Get());
            continue;
        }

        if(_cipher.IsEncrypted() && 0 <= av_packet_make_writable(packet.Get()))
            _cipher.Decrypt(std::span<u8>(packet->data, static_cast<uSize>(packet->size)));

        return 0 <= decoder.SendPacket(packet.Get());
    }

    return false;
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

    // the stream's own length, in its time_base
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
        const bool isLooping { _isLooping };

        // AVERROR_EOF at the end of the file - a read error ends the stream the same way. Looping, the end of the file is
        // where a loop over the whole of it (or past its end) goes back
        if(0 > _mediaSource.ReadPacket(packet.Get()))
        {
            if(isLooping && goBackToLoopStart())
                continue;

            if(isLooping)
                return; // aborted, or finished because the seek failed

            break;
        }

        if(packet->stream_index != _audioIndex)
            continue; // video, subtitles - not ours

        // A packet starting at or past the loop end isn't decoded: the loop goes back instead
        const i64 timestamp { packet->pts != AV_NOPTS_VALUE ? packet->pts : packet->dts };

        if(isLooping && timestamp != AV_NOPTS_VALUE && timestamp >= _loopEndPts)
        {
            if(!goBackToLoopStart())
                return;

            continue;
        }

        //Decrypted here, as FFmpeg's HCA decoder doesn't
        if(_cipher.IsEncrypted() && 0 <= av_packet_make_writable(packet.Get()))
            _cipher.Decrypt(std::span<u8>(packet->data, static_cast<uSize>(packet->size)));

        // false means the queue was aborted - a Stop or a seek, not the end of the file
        if(!_packetQueue.PushPacket(std::move(packet)))
            return;
    }

    _packetQueue.Finish();
}

bool RetroFuturaGUI::AudioStream::goBackToLoopStart()
{
    Packet marker;
    marker->stream_index = AudioDecodeThread::LoopMarker;

    if(!_packetQueue.PushPacket(std::move(marker)))
        return false;

    if(0 > _mediaSource.Seek(_loopSeekMilliseconds))
    {
        _packetQueue.Finish();
        return false;
    }

    return true;
}

void RetroFuturaGUI::AudioStream::SetLoop(const bool isLooping, const u64 startSample, const u64 endSample)
{
    const AVStream* stream { _mediaSource.GetStream(_audioIndex) };

    if(!stream)
        return;

    if(!stream->codecpar)
        return;

    if(0 >= stream->codecpar->sample_rate)
        return;

    //In the stream's time base, as packets count; an end of 0 is the end of the file, where no packet reaches
    const AVRational sampleRate { .num = 1, .den = stream->codecpar->sample_rate };
    const i64 startTime { stream->start_time == AV_NOPTS_VALUE ? 0 : stream->start_time };
    _loopEndPts = 0 == endSample ? INT64_MAX : av_rescale_q(static_cast<i64>(endSample), sampleRate, stream->time_base) + startTime;
    _loopSeekMilliseconds = std::max<i64>(0, av_rescale_q(static_cast<i64>(startSample), sampleRate, AVRational { .num = 1, .den = 1000 }) - _loopPrerollMilliseconds);
    _audioTrack.SetLoop(isLooping, startSample, endSample);
    _isLooping = isLooping;
}