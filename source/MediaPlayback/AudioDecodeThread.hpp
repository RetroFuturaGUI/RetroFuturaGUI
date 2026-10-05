#pragma once
#include <miniaudio.h>
#include "Resampler.hpp"
#include "config.hpp"
#include "Decoder.hpp"
#include "Frame.hpp"
#include "PacketQueue.hpp"
#include <atomic>
#include <thread>
#include <optional>

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
        
    private:
        void run();
        bool receiveFrames(Frame& frame);
        bool writeSamples(const uSize offset, const uSize frameCount);
        bool writeFrame(const Frame& frame);

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
            _ended { false };
    };
}
