#pragma once
#include "AudioMetadata.hpp"
#include "AudioSpectrum.hpp"
#include "AudioStream.hpp"
#include "ModuleSource.hpp"
#include "config.hpp"
#include <atomic>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
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
        ma_node_base _Base {};
        std::atomic<f32> _Levels[MaxMeterChannels] {};
        u32 _Channels { 0 };

        /// @brief The mono mix of everything that passed, for the spectrum. Written on the audio thread, read by
        /// GetFrequencyBands - miniaudio's ring buffer is lock-free for exactly one writer and one reader.
        ma_pcm_rb _Samples {};

        /// @brief Whether _Samples is initialized. Set before the node joins the graph and never changed while it is in it.
        bool _HasSamples { false };
    };

    /// @brief Plays audio files through miniaudio's high-level engine, which owns the
    /// output device, decoder, resampler and mixer. miniaudio decodes WAV, AIFF, FLAC and MP3
    /// itself, libopenmpt renders tracker modules (ModuleSource), and every other format is decoded by FFmpeg through AudioStream.
    /// @note ma_engine must keep a stable address and must never be copied, so this class
    /// is neither copyable nor movable.
    class AudioPlayback
    {
    public:
        /// @brief Joins the playbacks the Update loop keeps current (UpdateAll). Construct and destroy it on the windows' thread.
        AudioPlayback();
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

        /// @brief Loads a file, replacing whatever was loaded before, without starting it - StartPlaying()
        /// does that. The stream stays open so it can be sought, queried and stopped - unlike a
        /// fire-and-forget sound, which hands back no handle to act on. The file's own loop points are read too (see
        /// GetLoopPoints), and a file that has them is decoded by FFmpeg, even WAV, FLAC or MP3, so its loop has no gap.
        /// A tracker module (MOD, XM, S3M, IT, ...) plays through libopenmpt when the build has tracker modules
        /// (TrackerModulesEnabled), rendered at the device's rate.
        bool OpenAudioFile(std::string_view file);

        /// @brief Loads an encrypted HCA file with the game's key (see HcaKey) - or any file, the key going unused. Fails for
        ///        an encrypted file whose key doesn't fit, rather than playing noise.
        bool OpenAudioFile(std::string_view file, const HcaKey& key);

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
        /// @note An MP3 (decoded by miniaudio) has to be decoded whole to answer - so cache this, never call it per frame.
        u32 GetDuration() const;

        /// @brief Seeks to a position given in milliseconds.
        void Seek(const u32 position);

        void SetPlaybackSpeed(const f32 factor);

        /// @brief Plays in a loop while isLooping, without a gap: between the loop points (see GetLoopPoints), or the whole
        ///        file without any. A tracker module repeats from its own restart position, as its composer set it. Any time,
        ///        and it stays set for the next file. Off by default.
        void SetLooping(const bool isLooping);
        bool IsLooping() const;

        /// @brief Loops between two sample positions at the file's own rate instead of the file's loop points: the end
        ///        exclusive, 0 for the end of the file. Takes effect at once, also while looping. A file miniaudio decodes is
        ///        reopened through FFmpeg for it, at the same position. Lasts until the next file. False when nothing is
        ///        loaded, the end isn't after the start, or the file is a tracker module, which loops by its own patterns.
        bool SetLoopPoints(const u64 startSample, const u64 endSample);

        /// @brief The loop points SetLooping loops between: the file's own (from its header, smpl chunk or LOOPSTART tags),
        ///        or the ones SetLoopPoints set. None when the whole file loops.
        std::optional<AudioLoop> GetLoopPoints() const;

        /// @brief How many subsongs the loaded tracker module has - game music often keeps several in one file. 1 for any
        ///        other file, 0 with nothing loaded.
        uSize GetSubsongCount() const;

        /// @brief Switches the loaded tracker module to a subsong, from its start; -1 plays all of them one after another.
        ///        The position and duration follow the subsong. False for any other file.
        bool SelectSubsong(const i32 index);

        /// @brief Returns a channel's current output level as linear RMS, 0.0 to ~1.0.
        /// Measured after volume and pitch, with a fast attack and slow decay so it stays
        /// readable when polled once a frame. Returns 0 for channels the device does not
        /// have. Use 20*log10 for a dB display.
        f32 GetChannelVolume(const u32 channel) const;

        /// @brief Returns how many channels GetChannelVolume reports on.
        u32 GetChannelCount() const;

        /// @brief Analyzes the audio that arrived since the last call into the bands GetFrequencyBands points at. The Update
        /// loop calls it once per frame for every playback (UpdateAll). While nothing plays, the bands fall to 0.
        void UpdateFrequencyBands();

        /// @brief Updates the frequency bands of every playback alive. The first window's Update calls it once per frame.
        static void UpdateAll();

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
        /// @brief What decodes the loaded sound.
        enum class SoundDecoder
        {
            Miniaudio,  // WAV, AIFF, FLAC, MP3 without loop points
            Ffmpeg,     // everything else, through AudioStream
            Libopenmpt  // tracker modules, through ModuleSource
        };

        /// @brief Runs on the audio thread. Measures each channel into the meter's atomics and copies a
        /// mono mix into its ring buffer for the spectrum, then passes the audio through untouched - a
        /// meter must never colour what is heard.
        static void meterProcess(ma_node* node, const float** framesin, ma_uint32* framecountin, float** framesout, ma_uint32* framecountout);

        /// @brief Releases the loaded sound, if any. Idempotent.
        void unloadSound();

        /// @brief Loads the file as the sound, decoded by decoder.
        bool loadSound(std::string_view file, const HcaKey& key, const SoundDecoder decoder);

        /// @brief Hands the loop setting and points to whichever plays the sound.
        void applyLoop();

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
        ModuleSource _moduleSource {};
        std::string _filePath {}; //the loaded file, for reopening it through FFmpeg
        std::optional<AudioLoop> _loop {};
        u32
            _channels { 0 }, // 0 - let the device decide
            _sampleRate { 0 }; // 0 - let the device decide
        bool
            _initialized { false },
            _soundLoaded { false },
            _meterReady { false },
            _streamed { false },
            _isModule { false },
            _isLooping { false };
        static inline constexpr f32 _meterDecay { 0.85f }; //how smoothly the meter falls when the signal is falling
        static inline constexpr ma_uint32 _spectrumBufferFrameCount { 16384 };
        static inline std::vector<AudioPlayback*> _playbacks {}; //every playback alive, for UpdateAll
    };
}