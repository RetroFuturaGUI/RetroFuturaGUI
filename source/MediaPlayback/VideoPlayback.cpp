#include "VideoPlayback.hpp"
#include "VideoFrameQueue.hpp"
#include "VideoTexture.hpp"
#include <memory>
#include <algorithm>
#include <optional>

RetroFuturaGUI::VideoPlayback::VideoPlayback(AudioPlayback& output, Projection* projection)
    : _output(output)
{
    _texture = std::make_unique<VideoTexture>(projection);
}

RetroFuturaGUI::VideoPlayback::~VideoPlayback()
{
    Close();
}

bool RetroFuturaGUI::VideoPlayback::Open(std::string_view path)
{
    Close();

    if(!_mediaSource.Open(path))
        return false;

    _videoIndex = _mediaSource.FindBestStream(AVMEDIA_TYPE_VIDEO);

    if(-1 == _videoIndex)
    {
        Close();
        return false;
    }

    if(!_videoDecodeThread.Open(_mediaSource.GetStream(_videoIndex)))
    {
        Close();
        return false;
    }

    // sound is optional. a silent video refuses audio codecs
    _audioIndex = _mediaSource.FindBestStream(AVMEDIA_TYPE_AUDIO);

    if(-1 != _audioIndex)
    {
        if(!_audioTrack.Open(_mediaSource.GetStream(_audioIndex)) || !openSound())
        {
            _audioTrack.Close();
            _audioIndex = -1;
        }
    }

    _duration = readDuration();
    _clockBase = 0;
    _playing = false;
    _showNextPicture = true;
    _audioPackets.Reset();
    _videoPackets.Reset();
    startThreads();
    return true;
}

void RetroFuturaGUI::VideoPlayback::Close()
{
    stopThreads();

    if(_soundLoaded)
    {
        ma_sound_uninit(&_sound);
        _soundLoaded = false;
    }

    _audioTrack.Close();
    _mediaSource.Close();
    _audioIndex = -1;
    _videoIndex = -1;
    _duration = 0;
    _playing = false;
}

bool RetroFuturaGUI::VideoPlayback::IsOpen() const
{
    return -1 != _videoIndex;
}

bool RetroFuturaGUI::VideoPlayback::openSound()
{
    ma_engine* engine { _output.GetEngine() };
    ma_node* outputNode { _output.GetOutputNode() };

    if(!engine)
        return false;

    if(!outputNode)
        return false;

    if(ma_sound_init_from_data_source(engine, _audioTrack.GetDataSource(), MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, nullptr, &_sound) != MA_SUCCESS)
        return false;

    if(ma_node_attach_output_bus(&_sound, 0, outputNode, 0) != MA_SUCCESS)
    {
        ma_sound_uninit(&_sound);
        return false;
    }

    _soundLoaded = true;
    return true;
}

i64 RetroFuturaGUI::VideoPlayback::readDuration() const
{
    const AVFormatContext* formatContext { _mediaSource.GetFormatContext() };
    const AVStream* stream { _mediaSource.GetStream(_videoIndex) };

    if(!formatContext)
        return 0;

    if(!stream)
        return 0;

    if(formatContext->duration != AV_NOPTS_VALUE)
        return formatContext->duration / 1000;

    if(stream->duration != AV_NOPTS_VALUE)
        return av_rescale_q(stream->duration, stream->time_base, AVRational { .num = 1, .den = 1000 });

    return 0;
}

void RetroFuturaGUI::VideoPlayback::Play()
{
    if(!IsOpen())
        return;

    if(HasEnded())
    {
        Seek(0);
        _playing = false;
    }

    if(_playing)
        return;

    if(_soundLoaded)
        ma_sound_start(&_sound);

    _clockStart = std::chrono::steady_clock::now();
    _playing = true;
}

void RetroFuturaGUI::VideoPlayback::Pause()
{
    if(!_playing)
        return;

    _clockBase = GetPosition();

    if(_soundLoaded)
        ma_sound_stop(&_sound);

    _playing = false;
}

i64 RetroFuturaGUI::VideoPlayback::GetPosition() const
{
    if(HasAudio())
        return _audioTrack.GetPosition();

    i64 position { _clockBase };

    if(_playing)
        position += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _clockStart).count();

    return 0 < _duration ? std::min(position, _duration) : position;
}

bool RetroFuturaGUI::VideoPlayback::IsPlaying() const
{
    return _playing && !HasEnded();
}

bool RetroFuturaGUI::VideoPlayback::Seek(const i64 milliseconds)
{
    if(!IsOpen())
        return false;

    const i64 target { 0 < _duration ? std::clamp<i64>(milliseconds, 0, _duration) : std::max<i64>(0, milliseconds) };

    stopThreads();
    _audioPackets.Reset();
    _videoPackets.Reset();

    if(0 > _mediaSource.Seek(target))
    {
        startThreads();
        return false;
    }

    if(HasAudio())
        _audioTrack.SetStartPosition(target);

    _videoDecodeThread.SetStartPosition(target);
    _clockBase = target;
    _clockStart = std::chrono::steady_clock::now();
    _showNextPicture = true;
    startThreads();
    return true;
}

bool RetroFuturaGUI::VideoPlayback::HasEnded() const
{
    if(!IsOpen())
        return false;

    if(!_videoDecodeThread.HasEnded())
        return false;

    if(0 != _videoDecodeThread.GetFrameQueue().GetFrameCount())
        return false;

    if(_soundLoaded)
        return ma_sound_at_end(&_sound);

    return true;
}

void RetroFuturaGUI::VideoPlayback::Update()
{
    if(!IsOpen())
        return;

    if(!_texture)
        return;

    VideoFrameQueue& frames { _videoDecodeThread.GetFrameQueue() };
    const i64 clock { GetPosition() };
    VideoFrame due {};
    bool taken { false };

    // after Open and Seek the first picture shows at once, even while paused!
    if(_showNextPicture && frames.TryPop(due))
    {
        _showNextPicture = false;
        taken = true;
    }

    // the newest picture whose time has come. older ones  are skipped
    while(true)
    {
        const std::optional<i64> next { frames.PeekPosition() };

        if(!next || *next > clock)
            break;

        if(!frames.TryPop(due))
            break;

        taken = true;
    }

    if(taken)
        _texture->Update(due._Frame.Get());
}

RetroFuturaGUI::ITexture* RetroFuturaGUI::VideoPlayback::GetTexture()
{
    return _texture.get();
}

i64 RetroFuturaGUI::VideoPlayback::GetDuration() const
{
    return _duration;
}

bool RetroFuturaGUI::VideoPlayback::HasAudio() const
{
    return -1 != _audioIndex;
}

void RetroFuturaGUI::VideoPlayback::startThreads()
{
    _stopReading = false;

    if(HasAudio())
        _audioTrack.Start();

    _videoDecodeThread.Start();
    _readerThread = std::thread(&VideoPlayback::readPackets, this);
}

void RetroFuturaGUI::VideoPlayback::stopThreads()
{
    _stopReading = true;
    _audioTrack.Stop();
    _videoDecodeThread.Stop();

    if(_readerThread.joinable())
        _readerThread.join();
}

void RetroFuturaGUI::VideoPlayback::readPackets()
{
    while(true)
    {
        if(_stopReading)
            return;

        if(queuesHaveEnough())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        Packet packet;

        if(0 > _mediaSource.ReadPacket(packet.Get()))
            break; // AVERROR_EOF

        if(packet->stream_index == _videoIndex) // false means the queue was aborted
        {
            if(!_videoPackets.PushPacket(std::move(packet)))
                return;
        }
        else if(packet->stream_index == _audioIndex)
        {
            if(!_audioPackets.PushPacket(std::move(packet)))
                return;
        }
    }

    _audioPackets.Finish();
    _videoPackets.Finish();
}

bool RetroFuturaGUI::VideoPlayback::queuesHaveEnough() const
{
    constexpr uSize TotalByteLimit { 15 * 1024 * 1024 };
    constexpr uSize EnoughPacketsCount { 25 };

    if(TotalByteLimit < _audioPackets.GetByteSize() + _videoPackets.GetByteSize())
        return true;

    const bool audioHasEnough { !HasAudio() || EnoughPacketsCount <= _audioPackets.GetPacketCount() };
    const bool videoHasEnough { EnoughPacketsCount <= _videoPackets.GetPacketCount() };
    return audioHasEnough && videoHasEnough;
}