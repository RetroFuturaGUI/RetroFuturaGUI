#pragma once
#include "AudioSpectrum.hpp"
#include "AudioStream.hpp"
#include "config.hpp"
#include <atomic>
#include <span>
#include <string_view>
#include <miniaudio.h>

namespace RetroFuturaGUI
{
    /// @brief Highest channel count the level meter tracks.
    inline constexpr u32 MaxMeterChannels = 8;

    /// @brief A passthrough node that measures each channel's level as audio flows through
    /// it, and copies a mono mix out for the spectrum. Both happen on the audio thread inside
    /// onProcess; readers pick the levels up from the atomics and the samples from a lock-free
    /// ring buffer, so nothing that needs a lock is shared across threads.
    /// @note ma_node_base must stay the first member - miniaudio casts between the two.
    struct AudioMeterNode
    {
        ma_node_base base {};
        std::atomic<f32> levels[MaxMeterChannels] {};
        u32 channels { 0 };

        /// @brief The mono mix of everything that passed, for the spectrum. Written on the audio thread, read by
        /// GetFrequencyBands - miniaudio's ring buffer is lock-free for exactly one writer and one reader.
        ma_pcm_rb samples {};

        /// @brief Whether samples is initialized. Set before the node joins the graph and never changed while it is in it.
        bool hasSamples { false };
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

        /// @brief Analyzes the audio that arrived since the last call into the bands GetFrequencyBands points at. Call it
        /// once per frame on the thread that draws - Video::Draw does that for a video. While nothing plays, the bands fall to 0.
        void UpdateFrequencyBands();

        /// @brief The spectrum of what the meter measures (see GetChannelVolume): one level per frequency band, 0 to 1,
        /// spaced logarithmically from bass to treble. The levels change in place with every UpdateFrequencyBands, so a
        /// Histogram can be bound to this span once - it stays valid until SetFrequencyBandCount. All 0 without a meter.
        std::span<const f32> GetFrequencyBands() const;

        /// @brief Sets how many bands GetFrequencyBands reports - 32 by default.
        void SetFrequencyBandCount(const uSize bandCount);

        /// @brief The engine, for sounds that play alongside this one's - a video's, for instance.
        /// Null until InitDevice.
        ma_engine* GetEngine();

        /// @brief Where such sounds attach to be heard: the meter if there is one, so it measures them
        /// too, the engine's endpoint otherwise. Null until InitDevice.
        ma_node* GetOutputNode();

    private:
        /// @brief Runs on the audio thread. Measures each channel into the meter's atomics and copies a
        /// mono mix into its ring buffer for the spectrum, then passes the audio through untouched - a
        /// meter must never colour what is heard.
        static void meterProcess(ma_node* node, const float** framesin, ma_uint32* framecountin, float** framesout, ma_uint32* framecountout);

        /// @brief Releases the loaded sound, if any. Idempotent.
        void unloadSound();

        /// @brief Builds the meter node and wires it to the engine endpoint. Idempotent.
        bool initMeter();

        /// @brief Tears the meter node down. Idempotent.
        void uninitMeter();

        static bool isNativeFormat(std::string_view file);

        static void writeSpectrumSamples(RetroFuturaGUI::AudioMeterNode& meter, const f32* input, const u32 frameCount, const u32 channels);
        static void uninitSpectrumSamples(RetroFuturaGUI::AudioMeterNode& meter);

        ma_engine _engine {};
        ma_sound _sound {};
        AudioMeterNode _meter {};
        AudioSpectrum _spectrum {};
        AudioStream _audioStream {};
        u32
            _channels { 0 }, // 0 - let the device decide
            _sampleRate { 0 }; // 0 - let the device decide
        bool
            _initialized { false },
            _soundLoaded { false },
            _meterReady { false },
            _streamed { false };
        static inline constexpr f32 _meterDecay { 0.85f }; //how smoothly the meter falls when the signal is falling
        static inline constexpr ma_uint32 _spectrumBufferFrameCount { 16384 };
    };
}
