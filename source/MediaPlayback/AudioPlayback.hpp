#pragma once
#include "AudioStream.hpp"
#include "config.hpp"
#include <atomic>
#include <string_view>
#include <miniaudio.h>

namespace RetroFuturaGUI
{
    /// @brief Highest channel count the level meter tracks.
    inline constexpr u32 MaxMeterChannels = 8;

    /// @brief A passthrough node that measures each channel's level as audio flows through
    /// it. Measuring happens on the audio thread inside onProcess; readers pick the result
    /// up from the atomics, so no buffer is ever shared across threads.
    /// @note ma_node_base must stay the first member - miniaudio casts between the two.
    struct AudioMeterNode
    {
        ma_node_base base {};
        std::atomic<f32> levels[MaxMeterChannels] {};
        u32 channels { 0 };
    };

    /// @brief Plays audio files through miniaudio's high-level engine, which owns the
    /// output device, decoder, resampler and mixer. Handles WAV, FLAC and MP3 on its own;
    /// Opus and Vorbis need decoding through MediaPlayer first.
    /// @note ma_engine must keep a stable address and must never be copied, so this class
    /// is neither copyable nor movable.
    class AudioPlayback
    {
    public:
        AudioPlayback() = default;
        AudioPlayback(const AudioPlayback&) = delete;
        AudioPlayback(AudioPlayback&&) = delete;
        AudioPlayback& operator=(const AudioPlayback&) = delete;
        AudioPlayback& operator=(AudioPlayback&&) = delete;
        ~AudioPlayback();

        /// @brief Opens the default playback device and starts the engine. Idempotent.
        bool InitDevice();

        /// @brief Shuts the engine and its device down. Idempotent.
        void UninitDevice();

        /// @brief Resumes the engine after StopDevice().
        bool StartDevice();

        /// @brief Pauses the engine without tearing the device down.
        bool StopDevice();

        /// @brief Sets the channel count used on the next InitDevice(). 0 uses the device default.
        void SetPlaybackChannels(const u32 channels);

        /// @brief Sets the sample rate used on the next InitDevice(). 0 uses the device default.
        void SetSampleRate(const u32 sampleRate);

        /// @brief Loads a file and starts playing it, replacing whatever was loaded before.
        /// The stream stays open so it can be sought, queried and stopped - unlike a
        /// fire-and-forget sound, which hands back no handle to act on.
        bool OpenAudioFile(std::string_view file);

        /// @brief Stops playback and rewinds to the start. The file stays loaded, so
        /// StartPlaying() resumes from the beginning.
        void StopPlaying();

        /// @brief Resumes the loaded file from the current position.
        bool StartPlaying();

        /// @brief Returns whether a file is loaded and currently audible.
        bool IsPlaying() const;

        /// @brief Sets the master output volume. 1.0 is unity gain.
        bool SetVolume(const f32 volume);

        /// @brief Returns whether the engine holds an open device.
        bool IsInitialized() const { return _initialized; }

        /// @brief Returns the playback position in milliseconds, or 0 when nothing is loaded.
        u32 GetPosition() const;

        /// @brief Returns the loaded file's total length in milliseconds, 0 if unknown.
        /// @note Vorbis always reports 0 (an stb_vorbis push-mode limitation), and MP3
        /// has to decode the whole file to answer - so cache this, never call it per frame.
        u32 GetDuration() const;

        /// @brief Seeks to a position given in milliseconds.
        void Seek(const u32 position);

        void SetPlaybackSpeed(const f32 factor);

        /// @brief Returns a channel's current output level as linear RMS, 0.0 to ~1.0.
        /// Measured after volume and pitch, with a fast attack and slow decay so it stays
        /// readable when polled once a frame. Returns 0 for channels the device does not
        /// have. Use 20*log10 for a dB display.
        f32 GetChannelVolume(const u32 channel) const;

        /// @brief Returns how many channels GetChannelVolume reports on.
        u32 GetChannelCount() const;

        /// @brief The engine, for sounds that play alongside this one's - a video's, for instance.
        /// Null until InitDevice.
        ma_engine* GetEngine();

        /// @brief Where such sounds attach to be heard: the meter if there is one, so it measures them
        /// too, the engine's endpoint otherwise. Null until InitDevice.
        ma_node* GetOutputNode();

    private:
        /// @brief Runs on the audio thread. Measures each channel into the meter's atomics,
        /// then passes the audio through untouched - a meter must never colour what is heard.
        static void meterProcess(ma_node* node, const float** framesin, ma_uint32* framecountin, float** framesout, ma_uint32* framecountout);

        /// @brief Releases the loaded sound, if any. Idempotent.
        void unloadSound();

        /// @brief Builds the meter node and wires it to the engine endpoint. Idempotent.
        bool initMeter();

        /// @brief Tears the meter node down. Idempotent.
        void uninitMeter();

        static bool isNativeFormat(std::string_view file);

        ma_engine _engine {};
        ma_sound _sound {};
        AudioMeterNode _meter {};
        AudioStream _audioStream {};
        u32
            _channels { 0 }, // 0 - let the device decide
            _sampleRate { 0 }; // 0 - let the device decide
        bool
            _initialized { false },
            _soundLoaded { false },
            _meterReady { false },
            _streamed { false };
    };
}
