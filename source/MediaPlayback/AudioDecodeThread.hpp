#pragma once
#include <miniaudio.h>
#include "Resampler.hpp"
#include "config.hpp"
#include "Decoder.hpp"
#include "MediaFrame.hpp"
#include "PacketQueue.hpp"
#include <atomic>
#include <thread>
#include <optional>
#include <deque>
#include <mutex>

namespace RetroFuturaGUI
{
    /// @brief Decodes one audio stream on its own thread
    class AudioDecodeThread
    {
    public:
        explicit AudioDecodeThread(PacketQueue& queue);
        AudioDecodeThread(const AudioDecodeThread&) = delete;
        AudioDecodeThread(AudioDecodeThread&&) = delete;
        auto operator =(const AudioDecodeThread&) = delete;
        auto operator =(AudioDecodeThread&&) = delete;
        ~AudioDecodeThread();

        /// @brief Opens the decoder for the stream. Must happen before Start, and fails while running.
        /// Open again to restart from the beginning: Start only flushes, and a flush doesn't
        /// re-arm the codec's start delay (Opus' pre-skip), so the first run's samples would come out longer.
        bool Open(const AVStream* stream);

        /// @brief Starts decoding from wherever the queue's packets begin (after a seek, for instance).
        /// After a Stop, or once the thread has ended, call Stop first.
        void Start();

        /// @brief Stops the thread and waits for it
        void Stop();

        /// @brief True once every packet has been decoded and written. NOT AFTER STOP!
        bool HasEnded() const;

        ma_pcm_rb* GetRingBuffer();

        uSize GetWrittenFramesCount() const;

        /// @brief Frames before this position are dropped, and the frame it falls inside is cut,
        /// so the first sample written is exactly at the target. Only while stopped - before Start.
        void SetStartPosition(const i64 milliseconds);

        /// @brief A packet of this stream index in the queue marks a loop: the reader goes back to the loop start after it.
        static constexpr i32 LoopMarker { -1 };

        /// @brief Where a loop went back to its start: the frame of the ring where the loop start's audio begins.
        struct LoopWrap
        {
            uSize _WrittenFramesCount { 0 };
            u64 _StartSample { 0 };
        };

        /// @brief Loops between two sample positions (at the stream's rate, the end exclusive; an end of 0 is the end of the
        ///        file) while isLooping: nothing past the end is written, and at a LoopMarker the decoder is drained and goes
        ///        on from the start. Any time.
        void SetLoop(const bool isLooping, const u64 startSample, const u64 endSample);

        /// @brief The first loop restart the data source has read up to (readFramesCount), taken out of the list.
        std::optional<LoopWrap> TakeLoopWrap(const uSize readFramesCount) const;

        /// @brief Forgets the loop restarts recorded, for a seek. Only while stopped.
        void ClearLoopWraps();

    private:
        void run();
        bool receiveFrames(MediaFrame& frame);
        bool writeSamples(const uSize offset, const uSize frameCount);
        bool writeFrame(const MediaFrame& frame);

        /// @brief At a LoopMarker: the decoder's last frames written up to the loop end, the decoder reset, the restart
        ///        recorded, and everything before the loop start dropped from now on.
        bool jumpToLoopStart(MediaFrame& frame);
        i64 toPts(const u64 sample) const;

        PacketQueue& _queue;
        Decoder _decoder;
        std::thread _thread;
        std::atomic<uSize> _writtenFramesCount { 0 };
        Resampler _resampler {};
        ma_pcm_rb _ringBuffer {};
        std::optional<i64> _skipUntilPts {};
        AVRational _timeBase { .num = 0, .den = 1 };
        i64 _startTime { 0 };
        std::atomic<bool>
            _stop { false },
            _ended { false },
            _isLooping { false };
        std::atomic<u64>
            _loopStartSample { 0 },
            _loopEndSample { 0 };
        mutable std::deque<LoopWrap> _loopWraps {}; //taken off by the clock, which only reads otherwise
        mutable std::mutex _loopWrapsMutex {}; //the decode thread records restarts, the clock takes them
    };
}