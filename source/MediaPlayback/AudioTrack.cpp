#include "AudioTrack.hpp"

RetroFuturaGUI::AudioTrack::AudioTrack(PacketQueue& packetQueue)
    : _decodeThread(packetQueue)
{

}

RetroFuturaGUI::AudioTrack::~AudioTrack()
{
    Close();
}

bool RetroFuturaGUI::AudioTrack::Open(const AVStream* stream)
{
    if(!stream)
        return false;

    Close();

    if(!_decodeThread.Open(stream))
        return false;

    if(!_ringBufferSource.Open(_decodeThread))
        return false;

    _sampleRate = ma_pcm_rb_get_sample_rate(_decodeThread.GetRingBuffer());
    _baseFramesCount = 0;
    _basePosition = 0;
    return true;
}

void RetroFuturaGUI::AudioTrack::Close()
{
    _decodeThread.Stop();
    _ringBufferSource.Close();
    _sampleRate = 0;
}

bool RetroFuturaGUI::AudioTrack::IsOpen() const
{
    return 0 != _sampleRate;
}

void RetroFuturaGUI::AudioTrack::Start()
{
    _decodeThread.Start();
}

void RetroFuturaGUI::AudioTrack::Stop()
{
    _decodeThread.Stop();
}

void RetroFuturaGUI::AudioTrack::SetStartPosition(const i64 milliseconds)
{
    // The decode thread is stopped, so its count is final: everything up to it in the ring is from
    // before the seek. The data source drops it on its next read - even if that's after a long pause.
    const uSize writtenFramesCount { _decodeThread.GetWrittenFramesCount() };
    _ringBufferSource.DiscardUntil(writtenFramesCount);
    _baseFramesCount = writtenFramesCount;
    _basePosition = milliseconds;
    _decodeThread.SetStartPosition(milliseconds);
    _decodeThread.ClearLoopWraps(); // restarts of loops that will never play
}

void RetroFuturaGUI::AudioTrack::SetLoop(const bool isLooping, const u64 startSample, const u64 endSample)
{
    _decodeThread.SetLoop(isLooping, startSample, endSample);
}

ma_data_source* RetroFuturaGUI::AudioTrack::GetDataSource()
{
    return _ringBufferSource.Get();
}

i64 RetroFuturaGUI::AudioTrack::GetPosition() const
{
    if(0 == _sampleRate)
        return 0;

    const uSize readFramesCount { _ringBufferSource.GetReadFramesCount() };

    // Played past a loop restart: the clock goes on from the loop start
    while(const std::optional<AudioDecodeThread::LoopWrap> wrap { _decodeThread.TakeLoopWrap(readFramesCount) })
    {
        _baseFramesCount = wrap->_WrittenFramesCount;
        _basePosition = static_cast<i64>(wrap->_StartSample * 1000 / _sampleRate);
    }

    const uSize baseFramesCount { _baseFramesCount };

    if(readFramesCount <= baseFramesCount)
        return _basePosition;

    return _basePosition + static_cast<i64>((readFramesCount - baseFramesCount) * 1000 / _sampleRate);
}

bool RetroFuturaGUI::AudioTrack::HasEnded() const
{
    return _decodeThread.HasEnded();
}