#pragma once
#include "VideoDecodeThread.hpp"
#include "AudioPlayback.hpp"
#include "ITexture.hpp"
#include "AudioTrack.hpp"
#include "VideoTexture.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <string_view>
#include <thread>

namespace RetroFuturaGUI 
{
    class VideoPlayback
    {
    public:
        /// @param output The engine and meter the sound plays through.
        explicit VideoPlayback(AudioPlayback& output, Projection* projection = nullptr);
        // copy and move deleted: the threads, miniaudio and meshes hold pointers into it
        VideoPlayback(const VideoPlayback&) = delete;
        VideoPlayback(VideoPlayback&&) = delete;
        auto operator =(const VideoPlayback&) = delete;
        auto operator =(VideoPlayback&&) = delete;
        ~VideoPlayback();

        /// @brief Opens a file with a video stream and starts decoding, paused until Play.
        bool Open(std::string_view path);
        void Close();
        bool IsOpen() const;

        void Play();
        void Pause();
        bool IsPlaying() const;
        bool Seek(const i64 milliseconds);
        bool HasEnded() const;

        /// @brief Render thread, once per frame, before drawing: shows the picture that's due.
        void Update();

        ITexture* GetTexture();
        i64 GetPosition() const;
        i64 GetDuration() const;
        bool HasAudio() const;

    private:
        void startThreads();
        void stopThreads();
        void readPackets();
        bool queuesHaveEnough() const;
        bool openSound();
        i64 readDuration() const;

        AudioPlayback& _output;
        MediaSource _mediaSource {};
        PacketQueue
            _audioPackets { 16 * 1024 * 1024 },
            _videoPackets { 64 * 1024 * 1024 };
        AudioTrack _audioTrack { _audioPackets };
        VideoDecodeThread _videoDecodeThread { _videoPackets };
        std::unique_ptr<VideoTexture> _texture;
        ma_sound _sound {};
        std::thread _readerThread {};
        std::atomic<bool> _stopReading { false };
        i32
            _audioIndex { -1 },
            _videoIndex { -1 };
        i64 _duration { 0 };
        std::chrono::steady_clock::time_point _clockStart {};
        i64 _clockBase { 0 };
        bool
            _soundLoaded { false },
            _playing { false },
            _showNextPicture { true };
    };
}