#pragma once
#include "MediaSource.hpp"
#include "AudioTrack.hpp"
#include "HcaCipher.hpp"
#include "PacketQueue.hpp"
#include <string_view>
#include <atomic>
#include <cstdint>
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

        /// @brief Opens a file whose audio may be encrypted: an HCA file's blocks are decrypted with the key before they're
        ///        decoded. Fails when the first block doesn't decode with it - an encrypted file without its key, or a wrong one.
        bool Open(std::string_view path, const HcaKey& key);
        void Close();
        bool IsOpen() const;

        ma_data_source* GetDataSource();
        i64 GetPosition() const;
        i64 GetDuration() const;

        /// @brief Continues playback from a position in milliseconds, clamped to 0 and the duration.
        bool Seek(const i64 milliseconds);
        bool HasEnded() const;

        /// @brief Loops between two sample positions at the stream's rate (the end exclusive, 0 for the end of the file)
        ///        while isLooping, without a gap: the reader goes back to the loop start where the loop ends, and the clock
        ///        with it. Any time after Open; Close switches it off.
        void SetLoop(const bool isLooping, const u64 startSample, const u64 endSample);

    private:
        void startThreads();
        void stopThreads();
        void readPackets();
        i64 readDuration() const;

        /// @brief Puts a loop marker into the queue and seeks back before the loop start, so the decoder has warmed up
        ///        when the loop start's packets arrive. false when the queue was aborted, or the seek failed (the stream is then finished).
        bool goBackToLoopStart();

        /// @brief Whether the first audio block decodes once decrypted, read through a MediaSource of its own so the stream's
        ///        position stays at the start. An HCA block with a wrong key fails its checksum here.
        bool decodesFirstBlock(std::string_view path) const;

        MediaSource _mediaSource {};
        HcaCipher _cipher {}; //decrypts HCA blocks as the reader thread reads them
        PacketQueue _packetQueue { 256 * 1024 };
        AudioTrack _audioTrack { _packetQueue };
        std::thread _readerThread {};
        i32 _audioIndex { -1 };
        i64 _duration { 0 };
        std::atomic<bool> _isLooping { false };
        std::atomic<i64>
            _loopEndPts { INT64_MAX }, //packets from here on aren't decoded while looping
            _loopSeekMilliseconds { 0 }; //where the reader goes back to: a little before the loop start
        static constexpr i64 _loopPrerollMilliseconds { 100 };
    };
}