#pragma once
#include "config.hpp"
#include "Frame.hpp"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace RetroFuturaGUI
{
    /// @brief A decoded picture and when it is due.
    struct VideoFrame
    {
        Frame _Frame {};
        i64 _Position { 0 }; // ms from the start of the stream
    };

    /// @brief Decoded pictures between the video decode thread and the render thread
    class VideoFrameQueue
    {
    public:
        explicit VideoFrameQueue(const uSize capacity);
        VideoFrameQueue(const VideoFrameQueue&) = delete;
        VideoFrameQueue(VideoFrameQueue&&) = delete;
        ~VideoFrameQueue() = default;
        auto operator =(const VideoFrameQueue&) = delete;
        auto operator =(VideoFrameQueue&&) = delete;

        /// @brief Waits while the queue is full. Returns false once it has been aborted.
        bool Push(VideoFrame&& frame);

        /// @brief Takes the oldest picture without waiting. Returns false if there is none.
        bool TryPop(VideoFrame& frame);

        /// @brief The oldest picture's position, without taking it - empty if there is none.
        std::optional<i64> PeekPosition() const;

        /// @brief Wakes a waiting producer and makes every further Push fail.
        void Abort();

        /// @brief Drops every picture and lifts an abort. Safe while the consumer reads,
        /// but the producer has to be stopped.
        void Reset();

        uSize GetFrameCount() const;

    private:
        std::deque<VideoFrame> _frames {};
        mutable std::mutex _mutex {};
        std::condition_variable _notFull {};
        uSize _capacity { 0 };
        bool _aborted { false };
    };
}