#pragma once
#include "config.hpp"
#include <atomic>
#include <miniaudio.h>

namespace RetroFuturaGUI
{
    class AudioDecodeThread;

    /// @brief The miniaudio data source ma_sound reads from: takes the decode thread's samples out of its
    /// ring buffer, pads underruns with silence, reports the end of the stream, and counts the frames
    /// it hands out - the audio clock.
    class RingBufferSource
    {
    public:
        RingBufferSource() = default;
        RingBufferSource(const RingBufferSource&) = delete;
        RingBufferSource(RingBufferSource&&) = delete;
        ~RingBufferSource();
        RingBufferSource& operator=(const RingBufferSource&) = delete;
        RingBufferSource& operator=(RingBufferSource&&) = delete;

        /// @brief The decode thread has to be opened first: its ring buffer carries the format.
        bool Open(AudioDecodeThread& decodeThread);
        void Close();

        /// @brief What ma_sound_init_from_data_source takes.
        ma_data_source* Get();

        /// @brief Every frame taken out of the ring - handed to miniaudio or discarded after a seek -
        /// without the silence padded in on underruns.
        uSize GetReadFramesCount() const;

        /// @brief Drops what's in the ring until GetReadFramesCount() reaches frameCount. For a seek:
        /// pass the stopped decode thread's GetWrittenFramesCount(), and the old run's frames never play.
        void DiscardUntil(const uSize frameCount);

    private:
        static ma_result onRead(ma_data_source* dataSource, void* framesOut, ma_uint64 frameCount, ma_uint64* framesRead);
        static ma_result onGetDataFormat(ma_data_source* dataSource, ma_format* format, ma_uint32* channels, ma_uint32* sampleRate, ma_channel* channelMap, size_t channelMapCap);
        static ma_result onGetCursor(ma_data_source* dataSource, ma_uint64* cursor);

        ma_data_source_base _base {};
        AudioDecodeThread* _decodeThread { nullptr };
        std::atomic<uSize> _readFramesCount { 0 };
        std::atomic<uSize> _discardUntil { 0 };
    };
}
