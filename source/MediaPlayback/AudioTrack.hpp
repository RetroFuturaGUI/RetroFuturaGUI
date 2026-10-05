#pragma once
#include "config.hpp"
#include "AudioDecodeThread.hpp"
#include "PacketQueue.hpp"
#include "RingBufferSource.hpp"
#include <atomic>

namespace RetroFuturaGUI
{
    /// @brief The audio half of a playback: decodes one audio stream from a packet queue someone else
    /// fills, into a ring buffer that miniaudio reads through GetDataSource(), and keeps the clock.
    /// The file and the reader thread belong to the owner - AudioStream for audio files, VideoPlayback
    /// for the sound of a video - so both share this one implementation of the seek and clock logic.
    class AudioTrack
    {
    public:
        explicit AudioTrack(PacketQueue& packetQueue);
        AudioTrack(const AudioTrack&) = delete;
        AudioTrack(AudioTrack&&) = delete;
        ~AudioTrack();
        auto operator =(const AudioTrack&) = delete;
        auto operator =(AudioTrack&&) = delete;

        /// @brief Opens the decoder, the ring buffer and the data source for an audio stream. Doesn't start
        /// decoding - Start does. Fails while running.
        bool Open(const AVStream* stream);

        /// @brief Stops decoding and closes the data source. The sound reading it has to be gone first.
        void Close();
        bool IsOpen() const;

        /// @brief Starts decoding from wherever the packet queue's packets begin.
        void Start();

        /// @brief Stops decoding and waits for it. Aborts the packet queue, which also frees a reader
        /// blocked on it - the owner Resets the queue before the next Start.
        void Stop();

        /// @brief Restarts the track's timeline at a position, for a seek: drops the audio still buffered
        /// from before, trims what's decoded next to start exactly there, and sets the clock to it.
        /// Only while stopped, with the packets already coming from the new position.
        void SetStartPosition(const i64 milliseconds);

        /// @brief What ma_sound_init_from_data_source takes.
        ma_data_source* GetDataSource();

        /// @brief The clock: ms of audio handed to miniaudio, counted from the last start position.
        i64 GetPosition() const;

        /// @brief True once every packet has been decoded and written - not after a Stop.
        bool HasEnded() const;

    private:
        AudioDecodeThread _decodeThread;
        RingBufferSource _ringBufferSource {};
        u32 _sampleRate { 0 };
        std::atomic<i64> _basePosition { 0 };
        std::atomic<uSize> _baseFramesCount { 0 };
    };
}
