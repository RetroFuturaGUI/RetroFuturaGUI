#include "AudioDecodeThread.hpp"
#include "config.hpp"
#include <chrono>
#include <cstring>
#include <algorithm>

RetroFuturaGUI::AudioDecodeThread::AudioDecodeThread(PacketQueue& queue)
    : _queue(queue)
{}

RetroFuturaGUI::AudioDecodeThread::~AudioDecodeThread()
{
    Stop();
    ma_pcm_rb_uninit(&_ringBuffer);
}

bool RetroFuturaGUI::AudioDecodeThread::Open(const AVStream* stream)
{
    if(!stream)
        return false;

    if(_thread.joinable())
        return false;

    _writtenFramesCount = 0;

    if(!_decoder.Open(stream))
        return false;

    if(!_resampler.Open(stream))
        return false;

    ma_pcm_rb_uninit(&_ringBuffer);
    _ringBuffer = {};
    const ma_uint32 frames = _resampler.GetSampleRate() / 2;

    if(ma_pcm_rb_init(ma_format_f32, _resampler.GetChannels(), frames, nullptr, nullptr, &_ringBuffer) != MA_SUCCESS)
        return false;

    ma_pcm_rb_set_sample_rate(&_ringBuffer, _resampler.GetSampleRate());
    _timeBase = stream->time_base;
    _startTime = stream->start_time == AV_NOPTS_VALUE ? 0 : stream->start_time;
    _skipUntilPts.reset();
    ClearLoopWraps();
    return true;
}

void RetroFuturaGUI::AudioDecodeThread::Start()
{
    if(_thread.joinable())
        return;

    _stop = false;
    _ended = false;
    _decoder.Flush(); // drops frames a stopped run left buffered and leaves the end-of-stream state
    _thread = std::thread(&AudioDecodeThread::run, this);
}

uSize RetroFuturaGUI::AudioDecodeThread::GetWrittenFramesCount() const
{
    return _writtenFramesCount;
}

void RetroFuturaGUI::AudioDecodeThread::Stop()
{
    _stop = true;
    _queue.Abort(); // wakes the thread if it's waiting in PopPacket

    if(_thread.joinable())
        _thread.join();

    _ended = false;
}

bool RetroFuturaGUI::AudioDecodeThread::writeFrame(const MediaFrame& frame)
{
    if(!frame.IsValid())
        return false;

    if(_stop)
        return false;

    const i32 converted { _resampler.Convert(frame) };

    if(0 > converted)
        return false;

    const uSize convertedCount { static_cast<uSize>(converted) };
    const i64 pts { frame->best_effort_timestamp };
    const AVRational outputRate { .num = 1, .den = static_cast<i32>(_resampler.GetSampleRate()) };
    uSize
        first { 0 }, // the part of the frame written
        last { convertedCount };

    // Before a start position (a seek, or a loop's start): dropped, the frame it falls inside cut at the front. A frame
    // that can't be placed ends the skipping - better a little early than skipping forever
    if(_skipUntilPts && pts == AV_NOPTS_VALUE)
        _skipUntilPts.reset();

    if(_skipUntilPts)
    {
        const i64 target { *_skipUntilPts };
        const i64 end { pts + av_rescale_q(frame->nb_samples, AVRational { .num = 1, .den = frame->sample_rate }, _timeBase) };

        if(end <= target)
            return true;

        _skipUntilPts.reset(); // this frame reaches the target, nothing after it is skipped

        if(pts < target)
            first = std::min(static_cast<uSize>(av_rescale_q(target - pts, _timeBase, outputRate)), convertedCount);
    }

    // While looping, nothing past the loop end: the frame it falls inside cut at the back (an end of 0 is the file's own)
    const u64 loopEndSample { _loopEndSample };

    if(_isLooping && 0 != loopEndSample && pts != AV_NOPTS_VALUE)
    {
        const i64 loopEnd { toPts(loopEndSample) };

        if(pts >= loopEnd)
            return true;

        last = std::min(last, static_cast<uSize>(av_rescale_q(loopEnd - pts, _timeBase, outputRate)));
    }

    if(first >= last)
        return true;

    return writeSamples(first, last - first);
}

bool RetroFuturaGUI::AudioDecodeThread::jumpToLoopStart(MediaFrame& frame)
{
    // The decoder's buffered frames still belong before the loop end; then it starts afresh on the loop start's packets
    _decoder.SendPacket(nullptr);

    if(!receiveFrames(frame))
        return false;

    _decoder.Flush();
    const u64 loopStartSample { _loopStartSample };

    {
        const std::scoped_lock lock { _loopWrapsMutex };
        _loopWraps.push_back(LoopWrap { ._WrittenFramesCount = _writtenFramesCount, ._StartSample = loopStartSample });
    }

    _skipUntilPts = toPts(loopStartSample);
    return true;
}

i64 RetroFuturaGUI::AudioDecodeThread::toPts(const u64 sample) const
{
    return av_rescale_q(static_cast<i64>(sample), AVRational { .num = 1, .den = static_cast<i32>(_resampler.GetSampleRate()) }, _timeBase) + _startTime;
}

void RetroFuturaGUI::AudioDecodeThread::SetLoop(const bool isLooping, const u64 startSample, const u64 endSample)
{
    _loopStartSample = startSample;
    _loopEndSample = endSample;
    _isLooping = isLooping;
}

std::optional<RetroFuturaGUI::AudioDecodeThread::LoopWrap> RetroFuturaGUI::AudioDecodeThread::TakeLoopWrap(const uSize readFramesCount) const
{
    const std::scoped_lock lock { _loopWrapsMutex };

    if(_loopWraps.empty() || _loopWraps.front()._WrittenFramesCount > readFramesCount)
        return std::nullopt;

    const LoopWrap wrap { _loopWraps.front() };
    _loopWraps.pop_front();
    return wrap;
}

void RetroFuturaGUI::AudioDecodeThread::ClearLoopWraps()
{
    const std::scoped_lock lock { _loopWrapsMutex };
    _loopWraps.clear();
}

bool RetroFuturaGUI::AudioDecodeThread::writeSamples(const uSize offset, const uSize frameCount)
{
    const float* samples { _resampler.GetData() };
    const u32
        channels { _resampler.GetChannels() },
        total { static_cast<u32>(frameCount) };
    u32 written { 0 };

    while(written < total)
    {
        if(_stop)
            return false;

        ma_uint32 count { total - written };
        void* destination { nullptr };

        if(ma_pcm_rb_acquire_write(&_ringBuffer, &count, &destination) != MA_SUCCESS)
            return false;

        if(0 == count)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        std::memcpy(destination, samples + (offset + written) * channels, count * channels * sizeof(float));
        ma_pcm_rb_commit_write(&_ringBuffer, count);
        written += count;
        _writtenFramesCount += count;
    }

    return true;
}

void RetroFuturaGUI::AudioDecodeThread::run()
{
    Packet packet;
    MediaFrame frame;

    if(!frame.IsValid())
        return;

    while(_queue.PopPacket(packet))
    {
        // the reader went back to the loop start after this
        if(LoopMarker == packet->stream_index)
        {
            if(!jumpToLoopStart(frame))
                return;

            continue;
        }

        // only skip corrupt packets rather than ending playback
        if(0 > _decoder.SendPacket(packet.Get()))
            continue;

        if(!receiveFrames(frame))
            return; // stopped while writing
    }

    // PopPacket returns false on an abort too - that isn't the end of the stream
    if(_stop || _queue.IsAborted())
        return;

    // hand the decoder the end-of-stream marker and collect its last buffered frames
    _decoder.SendPacket(nullptr);

    if(!receiveFrames(frame))
        return;

    const i32 flushed { _resampler.Flush() };

    if(0 < flushed && !writeSamples(0, static_cast<uSize>(flushed)))
        return;

    _ended = true;
}

bool RetroFuturaGUI::AudioDecodeThread::receiveFrames(MediaFrame& frame)
{
    // collect frames until the decoder needs more input
    while(0 == _decoder.ReceiveFrame(frame.Get()))
    {
        if(!writeFrame(frame))
            return false;
    }

    return true;
}

bool RetroFuturaGUI::AudioDecodeThread::HasEnded() const 
{ 
    return _ended; 
}

ma_pcm_rb* RetroFuturaGUI::AudioDecodeThread::GetRingBuffer()
{
    return &_ringBuffer;
}

void RetroFuturaGUI::AudioDecodeThread::SetStartPosition(const i64 milliseconds)
{
    _skipUntilPts = av_rescale_q(milliseconds, AVRational { .num = 1, .den = 1000 }, _timeBase) + _startTime;
}