#include "AudioPlayback.hpp"
#include <cmath>
#include <cstring>
#include <string>

// How much of the previous level survives a block while the signal is falling. A raw
// per-block RMS flickers far too fast to read, so the meter rises instantly and falls
// gently. Block-rate dependent, which is fine for a display.
static constexpr f32 MeterDecay { 0.85f };

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
    const u32 channels = meter->channels;
    const f32* input = framesin[0];

    for(u32 ch = 0; ch < channels; ++ch)
    {
        f32 sumofsquares = 0.0f;

        for(u32 i = 0; i < framecount; ++i)
        {
            const f32 sample = input[i * channels + ch];
            sumofsquares += sample * sample;
        }

        const f32 rms = framecount > 0 ? std::sqrt(sumofsquares / static_cast<f32>(framecount)) : 0.0f;
        const f32 previous = meter->levels[ch].load(std::memory_order_relaxed);
        const f32 smoothed = rms > previous ? rms : previous * MeterDecay;

        meter->levels[ch].store(smoothed, std::memory_order_relaxed);
    }

    std::memcpy(framesout[0], input, static_cast<size_t>(framecount) * channels * sizeof(f32));

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

    if(channels == 0)
        return false;

    if(channels > MaxMeterChannels)
        channels = MaxMeterChannels;

    _meter.channels = channels;

    for(u32 ch = 0; ch < MaxMeterChannels; ++ch)
        _meter.levels[ch].store(0.0f, std::memory_order_relaxed);

    ma_node_config config = ma_node_config_init();
    config.vtable = &metervtable;
    config.pInputChannels = &channels;
    config.pOutputChannels = &channels;

    if(ma_node_init(ma_engine_get_node_graph(&_engine), &config, nullptr, &_meter.base) != MA_SUCCESS)
        return false;

    if(ma_node_attach_output_bus(&_meter.base, 0, ma_engine_get_endpoint(&_engine), 0) != MA_SUCCESS)
    {
        ma_node_uninit(&_meter.base, nullptr);
        return false;
    }

    _meterReady = true;
    return true;
}

void RetroFuturaGUI::AudioPlayback::uninitMeter()
{
    if(!_meterReady)
        return;

    ma_node_uninit(&_meter.base, nullptr);
    _meterReady = false;
}

RetroFuturaGUI::AudioPlayback::~AudioPlayback()
{
    UninitDevice();
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
    return true;
}

void RetroFuturaGUI::AudioPlayback::UninitDevice()
{
    if(!_initialized)
        return;

    // Teardown runs against the signal flow: sound feeds the meter, meter feeds the endpoint, so each must go before the thing it was attached to.
    unloadSound();
    uninitMeter();
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
    if(!_initialized)
        return false;

    unloadSound();
    ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;

    if(_meterReady)
        flags |= MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT;

    if(isNativeFormat(file))
    {
        const std::string filepath(file); //need 0-terminated string

        if(ma_sound_init_from_file(&_engine, filepath.c_str(), flags | MA_SOUND_FLAG_STREAM, nullptr, nullptr, &_sound) != MA_SUCCESS)
            return false;
    }
    else
    {
        if(!_audioStream.Open(file))
            return false;

        if(ma_sound_init_from_data_source(&_engine, _audioStream.GetDataSource(), flags, nullptr, &_sound) != MA_SUCCESS)
        {
            _audioStream.Close();
            return false;
        }

        _streamed = true;
    }

    _soundLoaded = true;

    if(_meterReady && ma_node_attach_output_bus(&_sound, 0, &_meter.base, 0) != MA_SUCCESS)
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
    _soundLoaded = false;
    _streamed = false;
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

    if(factor <= 0.0f)
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
        return &_meter.base;

    return ma_engine_get_endpoint(&_engine);
}

f32 RetroFuturaGUI::AudioPlayback::GetChannelVolume(const u32 channel) const
{
    if(!_meterReady)
        return 0.0f;

    if(channel >= _meter.channels)
        return 0.0f;

    return _meter.levels[channel].load(std::memory_order_relaxed);
}

u32 RetroFuturaGUI::AudioPlayback::GetChannelCount() const
{
    if(!_meterReady)
        return 0;

    return _meter.channels;
}

bool RetroFuturaGUI::AudioPlayback::isNativeFormat(std::string_view file)
{
    RetroFuturaGUI::MediaSource source;

    if(!source.Open(file))
        return false;

    const AVFormatContext* context { source.GetFormatContext() };
    const AVStream* stream { source.GetStream(source.FindBestStream(AVMEDIA_TYPE_AUDIO)) };

    if(!context->iformat)
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

    if(container == "mp3")
        return codec == AV_CODEC_ID_MP3; // FFmpeg's mp3 demuxer also reads MP1/MP2 files - those go through the policy

    return false;
}