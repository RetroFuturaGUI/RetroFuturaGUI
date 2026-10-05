#include "VideoDecodeThread.hpp"

RetroFuturaGUI::VideoDecodeThread::VideoDecodeThread(PacketQueue& packetQueue)
    : _packetQueue(packetQueue)
{}

RetroFuturaGUI::VideoDecodeThread::~VideoDecodeThread()
{
    Stop();
}

bool RetroFuturaGUI::VideoDecodeThread::Open(const AVStream* stream)
{
    if(!stream)
        return false;

    if(!stream->codecpar)
        return false;

    if(_thread.joinable())
        return false;

    if(stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
        return false;

    if(!_decoder.Open(stream))
        return false;

    _timeBase = stream->time_base;
    _startTime = stream->start_time == AV_NOPTS_VALUE ? 0 : stream->start_time;
    _skipUntilTimeStamp.reset();
    _lastPosition = 0;
    _frameQueue.Reset();
    return true;
}

void RetroFuturaGUI::VideoDecodeThread::Start()
{
    if(_thread.joinable())
        return;

    _stop = false;
    _ended = false;
    _decoder.Flush();
    _frameQueue.Reset();
    _thread = std::thread(&VideoDecodeThread::run, this);
}

void RetroFuturaGUI::VideoDecodeThread::Stop()
{
    _stop = true;
    _packetQueue.Abort();
    _frameQueue.Abort();

    if(_thread.joinable())
        _thread.join();

    _ended = false; // a stopped run hasn't ended
}

bool RetroFuturaGUI::VideoDecodeThread::HasEnded() const
{
    return _ended;
}

void RetroFuturaGUI::VideoDecodeThread::SetStartPosition(const i64 milliseconds)
{
    _skipUntilTimeStamp = av_rescale_q(milliseconds, AVRational { .num = 1, .den = 1000 }, _timeBase) + _startTime;
}

const RetroFuturaGUI::VideoFrameQueue& RetroFuturaGUI::VideoDecodeThread::GetFrameQueue() const
{
    return _frameQueue;
}

RetroFuturaGUI::VideoFrameQueue& RetroFuturaGUI::VideoDecodeThread::GetFrameQueue()
{
    return _frameQueue;
}

void RetroFuturaGUI::VideoDecodeThread::run()
{
    Packet packet;

    while(_packetQueue.PopPacket(packet))
    {
        // only skip corrupt packets rather than ending playback
        if(0 > _decoder.SendPacket(packet.Get()))
            continue;

        if(!receiveFrames())
            return; // stopped while queueing
    }

    // PopPacket returns false on an abort too - that isn't the end of the stream
    if(_stop || _packetQueue.IsAborted())
        return;

    // hand the decoder the end-of-stream marker and collect the pictures it still holds - with frame threads that's several
    _decoder.SendPacket(nullptr);

    if(!receiveFrames())
        return;

    _ended = true;
}

bool RetroFuturaGUI::VideoDecodeThread::receiveFrames()
{
    while(true)
    {
        Frame frame;

        if(!frame.IsValid())
            return false;

        if(0 != _decoder.ReceiveFrame(frame.Get()))
            return true; // needs the next packet, or has nothing more after the end-of-stream marker

        if(!writeFrame(std::move(frame)))
            return false;
    }
}

bool RetroFuturaGUI::VideoDecodeThread::writeFrame(Frame&& frame)
{
    if(!frame.IsValid())
        return false;

    if(_stop)
        return false;

    const i64 presentationTimeStamp { frame->best_effort_timestamp };

    // a picture without a timestamp keeps the last position: shown right after its predecessor
    if(presentationTimeStamp != AV_NOPTS_VALUE)
        _lastPosition = av_rescale_q(presentationTimeStamp - _startTime, _timeBase, AVRational { .num = 1, .den = 1000 });

    if(_skipUntilTimeStamp && presentationTimeStamp != AV_NOPTS_VALUE)
    {
        const i64 target { *_skipUntilTimeStamp };

        // A picture stays on screen until the next one, so the one showing at the target is the last
        // that starts at or before it. With its duration known that's exact; without, every picture
        // that starts before the target goes.
        const bool before { 0 < frame->duration ? presentationTimeStamp + frame->duration <= target : presentationTimeStamp < target };

        if(before)
            return true; // dropped, the queue never sees it

        _skipUntilTimeStamp.reset(); // reached - everything after this is queued
    }

    return _frameQueue.Push(VideoFrame { ._Frame = std::move(frame), ._Position = _lastPosition });
}