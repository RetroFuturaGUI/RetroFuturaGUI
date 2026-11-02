#include "AudioPlayback.hpp"
#include "AudioMetadataReader.hpp"
#include <cmath>
#include <cstring>
#include <string>

void RetroFuturaGUI::AudioPlayback::writeSpectrumSamples(RetroFuturaGUI::AudioMeterNode& meter, const f32* input, const u32 frameCount, const u32 channels)
{
    if(!input)
        return;

    if(0 == channels)
        return;

    const f32 channelWeight { 1.0f / static_cast<f32>(channels) };
    u32 writtenFrameCount { 0 };

    while(writtenFrameCount < frameCount)
    {
        ma_uint32 acquiredFrameCount { frameCount - writtenFrameCount };
        void* buffer { nullptr };

        // Hands out the free part up to the ring's end at most - two pieces when the write wraps around - and nothing when full
        if(ma_pcm_rb_acquire_write(&meter._Samples, &acquiredFrameCount, &buffer) != MA_SUCCESS)
            return;

        if(0 == acquiredFrameCount)
            return;

        f32* mono { static_cast<f32*>(buffer) };

        for(u32 frameIndex = 0; frameIndex < acquiredFrameCount; ++frameIndex)
        {
            const f32* frame { input + static_cast<uSize>(writtenFrameCount + frameIndex) * channels };
            f32 sum { 0.0f };

            for(u32 channel = 0; channel < channels; ++channel)
                sum += frame[channel];

            mono[frameIndex] = sum * channelWeight;
        }

        ma_pcm_rb_commit_write(&meter._Samples, acquiredFrameCount);
        writtenFrameCount += acquiredFrameCount;
    }
}

void RetroFuturaGUI::AudioPlayback::uninitSpectrumSamples(RetroFuturaGUI::AudioMeterNode& meter)
{
    if(!meter._HasSamples)
        return;

    ma_pcm_rb_uninit(&meter._Samples);
    meter._HasSamples = false;
}

void RetroFuturaGUI::AudioPlayback::meterProcess(ma_node* node, const float** framesin, ma_uint32* framecountin, float** framesout, ma_uint32* framecountout)
{
    AudioMeterNode* meter = reinterpret_cast<AudioMeterNode*>(node);

    if(!meter)
        return;

    if(!framecountin)
        return;

    if(!framecountout)
        return;

    if(!framesout || !framesout[0])
        return;

    if(!framesin || !framesin[0])
    {
        *framecountin = 0;
        *framecountout = 0;
        return;
    }

    const u32 framecount = *framecountin < *framecountout ? *framecountin : *framecountout;
    const u32 channels = meter->_Channels;
    const f32* input = framesin[0];

    for(u32 ch = 0; ch < channels; ++ch)
    {
        f32 sumofsquares = 0.0f;

        for(u32 i = 0; i < framecount; ++i)
        {
            const f32 sample = input[i * channels + ch];
            sumofsquares += sample * sample;
        }

        const f32 rms = 0 < framecount ? std::sqrt(sumofsquares / static_cast<f32>(framecount)) : 0.0f;
        const f32 previous = meter->_Levels[ch].load(std::memory_order_relaxed);
        const f32 smoothed = rms > previous ? rms : previous * _meterDecay;

        meter->_Levels[ch].store(smoothed, std::memory_order_relaxed);
    }

    if(meter->_HasSamples)
        writeSpectrumSamples(*meter, input, framecount, channels);

    std::memcpy(framesout[0], input, static_cast<uSize>(framecount) * channels * sizeof(f32));

    *framecountin = framecount;
    *framecountout = framecount;
}

bool RetroFuturaGUI::AudioPlayback::initMeter()
{
    if(_meterReady)
        return true;

    if(!_initialized)
        return false;

    // Declared here so it can reach the private callback.
    static constexpr ma_node_vtable metervtable
    {
        .onProcess = meterProcess,
        .onGetRequiredInputFrameCount = nullptr,
        .inputBusCount = 1,
        .outputBusCount = 1,
        .flags = 0
    };

    ma_uint32 channels = ma_engine_get_channels(&_engine);

    if(0 == channels)
        return false;

    if(channels > MaxMeterChannels)
        channels = MaxMeterChannels;

    _meter._Channels = channels;

    for(u32 ch = 0; ch < MaxMeterChannels; ++ch)
        _meter._Levels[ch].store(0.0f, std::memory_order_relaxed);

    // Optional as well: without it the spectrum stays empty and the levels still work. Made before the node
    // joins the graph, so the audio thread never sees it half-built.
    _meter._HasSamples = ma_pcm_rb_init(ma_format_f32, 1, _spectrumBufferFrameCount, nullptr, nullptr, &_meter._Samples) == MA_SUCCESS;

    ma_node_config config = ma_node_config_init();
    config.vtable = &metervtable;
    config.pInputChannels = &channels;
    config.pOutputChannels = &channels;

    if(ma_node_init(ma_engine_get_node_graph(&_engine), &config, nullptr, &_meter._Base) != MA_SUCCESS)
    {
        uninitSpectrumSamples(_meter);
        return false;
    }

    if(ma_node_attach_output_bus(&_meter._Base, 0, ma_engine_get_endpoint(&_engine), 0) != MA_SUCCESS)
    {
        ma_node_uninit(&_meter._Base, nullptr);
        uninitSpectrumSamples(_meter);
        return false;
    }

    _meterReady = true;
    return true;
}

void RetroFuturaGUI::AudioPlayback::uninitMeter()
{
    if(!_meterReady)
        return;

    ma_node_uninit(&_meter._Base, nullptr);

    // After the node: ma_node_uninit detaches it and waits for the audio thread to let go, so nothing writes anymore
    uninitSpectrumSamples(_meter);
    _meterReady = false;
}

RetroFuturaGUI::AudioPlayback::AudioPlayback()
{
    _playbacks.push_back(this);
}

RetroFuturaGUI::AudioPlayback::~AudioPlayback()
{
    std::erase(_playbacks, this);
    UninitDevice();
}

void RetroFuturaGUI::AudioPlayback::UpdateAll()
{
    for(AudioPlayback* playback : _playbacks)
    {
        if(!playback)
            continue;

        playback->UpdateFrequencyBands();
    }
}

bool RetroFuturaGUI::AudioPlayback::InitDevice()
{
    if(_initialized)
        return true;

    ma_engine_config config = ma_engine_config_init();
    config.channels = _channels;
    config.sampleRate = _sampleRate;

    if(ma_engine_init(&config, &_engine) != MA_SUCCESS)
        return false;

    _initialized = true;

    // optional. playback still works if the node cannot be created, the levels just stay at zero.
    initMeter();

    _spectrum.Init(ma_engine_get_sample_rate(&_engine));
    return true;
}

void RetroFuturaGUI::AudioPlayback::UninitDevice()
{
    if(!_initialized)
        return;

    // Teardown runs against the signal flow: sound feeds the meter, meter feeds the endpoint, so each must go before the thing it was attached to.
    unloadSound();
    uninitMeter();
    _spectrum.Uninit();
    ma_engine_uninit(&_engine);
    _initialized = false;
}

bool RetroFuturaGUI::AudioPlayback::StartDevice()
{
    if(!_initialized)
        return false;

    return ma_engine_start(&_engine) == MA_SUCCESS;
}

bool RetroFuturaGUI::AudioPlayback::StopDevice()
{
    if(!_initialized)
        return false;

    return ma_engine_stop(&_engine) == MA_SUCCESS;
}

void RetroFuturaGUI::AudioPlayback::SetPlaybackChannels(const u32 channels)
{
    _channels = channels;
}

void RetroFuturaGUI::AudioPlayback::SetSampleRate(const u32 sampleRate)
{
    _sampleRate = sampleRate;
}

bool RetroFuturaGUI::AudioPlayback::OpenAudioFile(std::string_view file)
{
    return OpenAudioFile(file, HcaKey {});
}

bool RetroFuturaGUI::AudioPlayback::OpenAudioFile(std::string_view file, const HcaKey& key)
{
    if(!_initialized)
        return false;

    unloadSound();
    _loop.reset();
    SoundDecoder decoder { SoundDecoder::Libopenmpt };

    // A tracker module repeats from its own restart position, so it has no loop points to read
    if(!ModuleSource::IsModule(file))
    {
        const std::optional<AudioMetadata> metadata { AudioMetadataReader::Read(file) };
        _loop = metadata ? metadata->_Loop : std::nullopt;

        // miniaudio's streamed sounds loop only over the whole file without a gap, so loop points go through FFmpeg
        decoder = _loop || !isNativeFormat(file) ? SoundDecoder::Ffmpeg : SoundDecoder::Miniaudio;
    }

    if(!loadSound(file, key, decoder))
        return false;

    applyLoop();
    return true;
}

bool RetroFuturaGUI::AudioPlayback::loadSound(std::string_view file, const HcaKey& key, const SoundDecoder decoder)
{
    ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;

    if(_meterReady)
        flags |= MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT;

    if(SoundDecoder::Miniaudio == decoder)
    {
        const std::string filepath(file); //need 0-terminated string

        if(ma_sound_init_from_file(&_engine, filepath.c_str(), flags | MA_SOUND_FLAG_STREAM, nullptr, nullptr, &_sound) != MA_SUCCESS)
            return false;
    }
    else if(SoundDecoder::Ffmpeg == decoder)
    {
        if(!_audioStream.Open(file, key))
            return false;

        if(ma_sound_init_from_data_source(&_engine, _audioStream.GetDataSource(), flags, nullptr, &_sound) != MA_SUCCESS)
        {
            _audioStream.Close();
            return false;
        }

        _streamed = true;
    }
    else
    {
        // Rendered at the engine's rate, so miniaudio has nothing to resample
        if(!_moduleSource.Open(file, ma_engine_get_sample_rate(&_engine)))
            return false;

        if(ma_sound_init_from_data_source(&_engine, _moduleSource.Get(), flags, nullptr, &_sound) != MA_SUCCESS)
        {
            _moduleSource.Close();
            return false;
        }

        _isModule = true;
    }

    _soundLoaded = true;
    _filePath = file;

    if(_meterReady && ma_node_attach_output_bus(&_sound, 0, &_meter._Base, 0) != MA_SUCCESS)
    {
        unloadSound();
        return false;
    }

    return true;
}

void RetroFuturaGUI::AudioPlayback::unloadSound()
{
    if(!_soundLoaded)
        return;

    ma_sound_uninit(&_sound);
    _audioStream.Close();
    _moduleSource.Close();
    _soundLoaded = false;
    _streamed = false;
    _isModule = false;
}

void RetroFuturaGUI::AudioPlayback::applyLoop()
{
    if(!_soundLoaded)
        return;

    if(!_streamed)
    {
        ma_sound_set_looping(&_sound, _isLooping ? MA_TRUE : MA_FALSE);
        return;
    }

    const AudioLoop loop { _loop.value_or(AudioLoop {}) }; //none: from 0 to the end of the file
    _audioStream.SetLoop(_isLooping, loop._StartSample, loop._EndSample);
}

void RetroFuturaGUI::AudioPlayback::SetLooping(const bool isLooping)
{
    _isLooping = isLooping;
    applyLoop();
}

bool RetroFuturaGUI::AudioPlayback::IsLooping() const
{
    return _isLooping;
}

bool RetroFuturaGUI::AudioPlayback::SetLoopPoints(const u64 startSample, const u64 endSample)
{
    if(!_soundLoaded)
        return false;

    if(0 != endSample && endSample <= startSample)
        return false;

    if(_isModule)
        return false; // it loops by its own patterns

    _loop = AudioLoop { ._StartSample = startSample, ._EndSample = endSample, ._Source = "SetLoopPoints" };

    // Reopened through FFmpeg where it left off, at the same speed
    if(!_streamed)
    {
        const u32 position { GetPosition() };
        const bool wasPlaying { IsPlaying() };
        const f32 pitch { ma_sound_get_pitch(&_sound) };
        const std::string file { _filePath };
        unloadSound();

        if(!loadSound(file, HcaKey {}, SoundDecoder::Ffmpeg))
            return false;

        ma_sound_set_pitch(&_sound, pitch);
        _audioStream.Seek(static_cast<i64>(position));

        if(wasPlaying)
            ma_sound_start(&_sound);
    }

    applyLoop();
    return true;
}

std::optional<RetroFuturaGUI::AudioLoop> RetroFuturaGUI::AudioPlayback::GetLoopPoints() const
{
    return _loop;
}

uSize RetroFuturaGUI::AudioPlayback::GetSubsongCount() const
{
    if(!_soundLoaded)
        return 0;

    if(!_isModule)
        return 1;

    return _moduleSource.GetSubsongCount();
}

bool RetroFuturaGUI::AudioPlayback::SelectSubsong(const i32 index)
{
    if(!_isModule)
        return false;

    return _moduleSource.SelectSubsong(index);
}

void RetroFuturaGUI::AudioPlayback::StopPlaying()
{
    if(!_soundLoaded)
        return;

    ma_sound_stop(&_sound);

    if(_streamed)
    {
        _audioStream.Seek(0);
        return;
    }

    ma_sound_seek_to_pcm_frame(&_sound, 0);
}

bool RetroFuturaGUI::AudioPlayback::StartPlaying()
{
    if(!_soundLoaded)
        return false;

    if(_streamed && ma_sound_at_end(&_sound) && _audioStream.HasEnded())
        _audioStream.Seek(0);

    return ma_sound_start(&_sound) == MA_SUCCESS;
}

bool RetroFuturaGUI::AudioPlayback::IsPlaying() const
{
    if(!_soundLoaded)
        return false;

    return ma_sound_is_playing(&_sound) == MA_TRUE;
}

u32 RetroFuturaGUI::AudioPlayback::GetPosition() const
{
    if(!_soundLoaded)
        return 0;

    if(_streamed)
        return static_cast<u32>(_audioStream.GetPosition());

    f32 cursorseconds = 0.0f;

    if(ma_sound_get_cursor_in_seconds(const_cast<ma_sound*>(&_sound), &cursorseconds) != MA_SUCCESS)
        return 0;

    return static_cast<u32>(cursorseconds * 1000.0f);
}

u32 RetroFuturaGUI::AudioPlayback::GetDuration() const
{
    if(!_soundLoaded)
        return 0;

    if(_streamed)
        return static_cast<u32>(_audioStream.GetDuration());

    f32 lengthseconds = 0.0f;
    
    if(ma_sound_get_length_in_seconds(const_cast<ma_sound*>(&_sound), &lengthseconds) != MA_SUCCESS)
        return 0;

    return static_cast<u32>(lengthseconds * 1000.0f);
}

void RetroFuturaGUI::AudioPlayback::Seek(const u32 position)
{
    if(!_soundLoaded)
        return;

    if(_streamed)
    {
        _audioStream.Seek(static_cast<i64>(position));
        return;
    }

    ma_sound_seek_to_second(&_sound, static_cast<f32>(position) * 0.001f);
}

bool RetroFuturaGUI::AudioPlayback::SetVolume(const f32 volume)
{
    if(!_initialized)
        return false;

    return ma_engine_set_volume(&_engine, volume) == MA_SUCCESS;
}

void RetroFuturaGUI::AudioPlayback::SetPlaybackSpeed(const f32 factor)
{
    if(!_soundLoaded)
        return;

    if(0.0f >= factor)
        return;

    ma_sound_set_pitch(&_sound, factor);
}

ma_engine* RetroFuturaGUI::AudioPlayback::GetEngine()
{
    if(!_initialized)
        return nullptr;

    return &_engine;
}

ma_node* RetroFuturaGUI::AudioPlayback::GetOutputNode()
{
    if(!_initialized)
        return nullptr;

    if(_meterReady)
        return &_meter._Base;

    return ma_engine_get_endpoint(&_engine);
}

f32 RetroFuturaGUI::AudioPlayback::GetChannelVolume(const u32 channel) const
{
    if(!_meterReady)
        return 0.0f;

    if(channel >= _meter._Channels)
        return 0.0f;

    return _meter._Levels[channel].load(std::memory_order_relaxed);
}

u32 RetroFuturaGUI::AudioPlayback::GetChannelCount() const
{
    if(!_meterReady)
        return 0;

    return _meter._Channels;
}

void RetroFuturaGUI::AudioPlayback::UpdateFrequencyBands()
{
    if(!_meterReady)
        return;

    if(!_meter._HasSamples)
        return;

    ma_uint32 remainingFrameCount { ma_pcm_rb_available_read(&_meter._Samples) };

    while(0 < remainingFrameCount)
    {
        ma_uint32 frameCount { remainingFrameCount };
        void* frames { nullptr };

        if(ma_pcm_rb_acquire_read(&_meter._Samples, &frameCount, &frames) != MA_SUCCESS)
            break;

        if(0 == frameCount)
            break;

        _spectrum.AddSamples(std::span<const f32>(static_cast<const f32*>(frames), frameCount));
        ma_pcm_rb_commit_read(&_meter._Samples, frameCount);
        remainingFrameCount -= frameCount;
    }

    _spectrum.Update();
}

std::span<const f32> RetroFuturaGUI::AudioPlayback::GetFrequencyBands() const
{
    return _spectrum.GetBands();
}

void RetroFuturaGUI::AudioPlayback::SetFrequencyBandCount(const uSize bandCount)
{
    _spectrum.SetBandCount(bandCount);
}

bool RetroFuturaGUI::AudioPlayback::isNativeFormat(std::string_view file)
{
    RetroFuturaGUI::MediaSource source;

    if(!source.Open(file))
        return false;

    const AVFormatContext* context { source.GetFormatContext() };
    const AVStream* stream { source.GetStream(source.FindBestStream(AVMEDIA_TYPE_AUDIO)) };

    if(!context)
        return false;

    if(!context->iformat)
        return false;

    if(!stream)
        return false;

    if(!stream->codecpar)
        return false;

    const std::string_view container { context->iformat->name };
    const AVCodecID codec { stream->codecpar->codec_id };

    if(container == "wav")
        return codec == AV_CODEC_ID_PCM_S16LE || codec == AV_CODEC_ID_PCM_S24LE
            || codec == AV_CODEC_ID_PCM_S32LE || codec == AV_CODEC_ID_PCM_F32LE || codec == AV_CODEC_ID_PCM_U8;

    if(container == "flac")
        return codec == AV_CODEC_ID_FLAC;

    //miniaudio's WAV decoder reads AIFF and AIFF-C too
    if(container == "aiff")
        return codec == AV_CODEC_ID_PCM_S8 || codec == AV_CODEC_ID_PCM_U8 || codec == AV_CODEC_ID_PCM_S16BE || codec == AV_CODEC_ID_PCM_S24BE
            || codec == AV_CODEC_ID_PCM_S32BE || codec == AV_CODEC_ID_PCM_S16LE || codec == AV_CODEC_ID_PCM_S24LE || codec == AV_CODEC_ID_PCM_S32LE
            || codec == AV_CODEC_ID_PCM_F32BE || codec == AV_CODEC_ID_PCM_F64BE || codec == AV_CODEC_ID_PCM_ALAW || codec == AV_CODEC_ID_PCM_MULAW;

    if(container == "mp3")
        return codec == AV_CODEC_ID_MP3; // FFmpeg's mp3 demuxer also reads MP1/MP2 files - those go through the policy

    return false;
}