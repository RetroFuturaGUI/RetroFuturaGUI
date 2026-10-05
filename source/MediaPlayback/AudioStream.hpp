#pragma once
#include "MediaSource.hpp"
#include "AudioTrack.hpp"
#include "PacketQueue.hpp"
#include <string_view>
#include <thread>

namespace RetroFuturaGUI
{
    /// @brief Plays the audio of a file through FFmpeg: owns the file and the reader thread that feeds
    /// an AudioTrack, which does the decoding, the ring buffer and the clock.
    class AudioStream
    {
    public:
        AudioStream() = default;
        AudioStream(const AudioStream&) = delete;
        AudioStream(AudioStream&&) = delete;
        ~AudioStream();
        auto operator =(const AudioStream&) = delete;
        auto operator =(AudioStream&&) = delete;

        bool Open(std::string_view path);
        void Close();
        bool IsOpen() const;

        ma_data_source* GetDataSource();
        i64 GetPosition() const;
        i64 GetDuration() const;

        /// @brief Continues playback from a position in milliseconds, clamped to 0 and the duration.
        bool Seek(const i64 milliseconds);
        bool HasEnded() const;

    private:
        void startThreads();
        void stopThreads();
        void readPackets();
        i64 readDuration() const;

        MediaSource _mediaSource {};
        PacketQueue _packetQueue { 256 * 1024 };
        AudioTrack _audioTrack { _packetQueue };
        std::thread _readerThread {};
        i32 _audioIndex { -1 };
        i64 _duration { 0 };
    };
}
