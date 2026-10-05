#pragma once
#include "config.hpp"
#include "Decoder.hpp"
#include "Frame.hpp"
#include "PacketQueue.hpp"
#include "VideoFrameQueue.hpp"
#include <atomic>
#include <optional>
#include <thread>

namespace RetroFuturaGUI
{
    /// @brief Decodes one video stream on its own thread
    class VideoDecodeThread
    {
    public:
        explicit VideoDecodeThread(PacketQueue& packetQueue);
        VideoDecodeThread(const VideoDecodeThread&) = delete;
        VideoDecodeThread(VideoDecodeThread&&) = delete;
        auto operator =(const VideoDecodeThread&) = delete;
        auto operator =(VideoDecodeThread&&) = delete;
        ~VideoDecodeThread();

        /// @brief Opens the decoder for a video stream. Must happen before Start, and fails while running.
        bool Open(const AVStream* stream);

        /// @brief Starts decoding from wherever the queue's packets begin. Drops the pictures a previous
        /// run left in the frame queue. After a Stop, or once the thread has ended, call Stop first.
        void Start();

        /// @brief Stops the thread and waits for it. Idempotent.
        void Stop();

        /// @brief True once every packet has been decoded and queued - not after a Stop.
        bool HasEnded() const;

        /// @brief Pictures that end before this position are dropped, so the first one queued is the
        /// picture showing at the target. Only while stopped - before Start.
        void SetStartPosition(const i64 milliseconds);

        /// @brief Where the decoded pictures come out. The render thread takes them from here.
        const VideoFrameQueue& GetFrameQueue() const;
        VideoFrameQueue& GetFrameQueue();

    private:
        void run();
        bool receiveFrames();
        bool writeFrame(Frame&& frame);

        PacketQueue& _packetQueue;
        VideoFrameQueue _frameQueue { 4 };
        Decoder _decoder {};
        std::thread _thread {};
        std::optional<i64> _skipUntilTimeStamp {};
        AVRational _timeBase { .num = 0, .den = 1 };
        i64
            _startTime { 0 },
            _lastPosition { 0 };
        std::atomic<bool>
            _stop { false },
            _ended { false };
    };
}
