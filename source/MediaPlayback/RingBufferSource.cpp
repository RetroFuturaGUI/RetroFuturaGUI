#include "RingBufferSource.hpp"
#include "AudioDecodeThread.hpp"
#include <algorithm>
#include <type_traits>

static_assert(std::is_standard_layout_v<RetroFuturaGUI::RingBufferSource>);

template<typename T>
static void setIfRequested(T* out, const T value)
{
    if(!out)
        return;

    *out = value;
}

RetroFuturaGUI::RingBufferSource::~RingBufferSource()
{
    Close();
}

void RetroFuturaGUI::RingBufferSource::DiscardUntil(const uSize frameCount)
{
    _discardUntil = frameCount;
}

bool RetroFuturaGUI::RingBufferSource::Open(AudioDecodeThread& decodeThread)
{
    static const ma_data_source_vtable vtable
    {
        .onRead = &RingBufferSource::onRead,
        .onSeek = nullptr,
        .onGetDataFormat = &RingBufferSource::onGetDataFormat,
        .onGetCursor = &RingBufferSource::onGetCursor,
        .onGetLength = nullptr,
        .onSetLooping = nullptr,
        .flags = 0
    };

    Close();

    ma_data_source_config config { ma_data_source_config_init() };
    config.vtable = &vtable;

    if(ma_data_source_init(&config, &_base) != MA_SUCCESS)
        return false;

    _decodeThread = &decodeThread;
    _readFramesCount = 0;
    _discardUntil = 0;
    return true;
}

void RetroFuturaGUI::RingBufferSource::Close()
{
    ma_data_source_uninit(&_base);
    _decodeThread = nullptr;
}

ma_data_source* RetroFuturaGUI::RingBufferSource::Get()
{
    return &_base;
}

uSize RetroFuturaGUI::RingBufferSource::GetReadFramesCount() const
{
    return _readFramesCount;
}

ma_result RetroFuturaGUI::RingBufferSource::onRead(ma_data_source* dataSource, void* framesOut, ma_uint64 frameCount, ma_uint64* framesRead)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    if(!framesOut)
        return MA_INVALID_ARGS;

    if(!framesRead)
        return MA_INVALID_ARGS;

    RingBufferSource* source { static_cast<RingBufferSource*>(dataSource) };

    if(!source->_decodeThread)
        return MA_INVALID_ARGS;

    // Before touching the ring: the decode thread sets _ended only after its last commit, so when it's
    // true here, everything it ever wrote is visible below. Checked afterwards, it could commit its last
    // frames and end in between - those would never be played.
    const bool ended { source->_decodeThread->HasEnded() };

    ma_pcm_rb* ringBuffer { source->_decodeThread->GetRingBuffer() };
    const ma_format format { ma_pcm_rb_get_format(ringBuffer) };
    const ma_uint32 channels { ma_pcm_rb_get_channels(ringBuffer) };
    ma_uint64 read { 0 };

    const uSize discardUntil { source->_discardUntil };

    while(source->_readFramesCount < discardUntil)
    {
        ma_uint32 count { static_cast<ma_uint32>(std::min<u64>(discardUntil - source->_readFramesCount, UINT32_MAX)) };
        void* samples { nullptr };

        if(MA_SUCCESS != ma_pcm_rb_acquire_read(ringBuffer, &count, &samples))
            break;

        if(0 == count)
            break;

        ma_pcm_rb_commit_read(ringBuffer, count);   // committed without copying - dropped
        source->_readFramesCount += count;
    }

    while(read < frameCount)
    {
        ma_uint32 count { static_cast<ma_uint32>(std::min<ma_uint64>(frameCount - read, UINT32_MAX)) };
        void* samples { nullptr };

        if(ma_pcm_rb_acquire_read(ringBuffer, &count, &samples) != MA_SUCCESS)
            break;

        if(count == 0)
            break;

        ma_copy_pcm_frames(ma_offset_pcm_frames_ptr(framesOut, read, format, channels), samples, count, format, channels);
        ma_pcm_rb_commit_read(ringBuffer, count);
        read += count;
    }

    source->_readFramesCount += read;

    if(read == frameCount)
    {
        *framesRead = read;
        return MA_SUCCESS;
    }

    if(ended)
    {
        *framesRead = read;
        return read > 0 ? MA_SUCCESS : MA_AT_END;
    }

    // Underrun - the decoder is behind. Pad with silence and report the full count
    ma_silence_pcm_frames(ma_offset_pcm_frames_ptr(framesOut, read, format, channels), frameCount - read, format, channels);
    *framesRead = frameCount;
    return MA_SUCCESS;
}

ma_result RetroFuturaGUI::RingBufferSource::onGetDataFormat(ma_data_source* dataSource, ma_format* format, ma_uint32* channels, ma_uint32* sampleRate, ma_channel* channelMap, size_t channelMapCap)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    const RingBufferSource* source { static_cast<const RingBufferSource*>(dataSource) };

    if(!source->_decodeThread)
        return MA_INVALID_ARGS;

    const ma_pcm_rb* ringBuffer { source->_decodeThread->GetRingBuffer() };
    const ma_uint32 channelCount { ma_pcm_rb_get_channels(ringBuffer) };

    setIfRequested(format, ma_pcm_rb_get_format(ringBuffer));
    setIfRequested(channels, channelCount);
    setIfRequested(sampleRate, ma_pcm_rb_get_sample_rate(ringBuffer));
    ma_channel_map_init_standard(ma_standard_channel_map_default, channelMap, channelMapCap, channelCount); // ignores a null map
    return MA_SUCCESS;
}

ma_result RetroFuturaGUI::RingBufferSource::onGetCursor(ma_data_source* dataSource, ma_uint64* cursor)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    if(!cursor)
        return MA_INVALID_ARGS;

    *cursor = static_cast<const RingBufferSource*>(dataSource)->_readFramesCount;
    return MA_SUCCESS;
}
