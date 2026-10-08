#pragma once
#include "config.hpp"
#include <chrono>
#include <span>
#include <vector>

extern "C"
{
    #include <libavutil/tx.h>
}

namespace RetroFuturaGUI
{
    /// @brief Turns mono-sampled audio into frequency bands ranging from 0 to 1
    /// @note Not thread-safe, and too much work for the audio thread: AudioPlayback collects the samples there and feeds
    /// them in on the thread that reads the bands.
    class AudioSpectrum
    {
    public:
        /// @brief How many samples one analysis covers. At 48 kHz that is about 43 ms, with about 23 Hz between frequency bins.
        static constexpr uSize WindowSize { 2048 };

        AudioSpectrum();
        AudioSpectrum(const AudioSpectrum&) = delete;
        AudioSpectrum(AudioSpectrum&&) = delete;
        AudioSpectrum& operator=(const AudioSpectrum&) = delete;
        AudioSpectrum& operator=(AudioSpectrum&&) = delete;
        ~AudioSpectrum();

        /// @brief Sets the transform up for samples at sampleRate, starting from silence.
        bool Init(const u32 sampleRate);

        /// @brief Frees the transform and forgets all samples. Idempotent.
        void Uninit();

        /// @brief Sets how many bands GetBands reports - 32 by default. Invalidates spans GetBands returned before.
        void SetBandCount(const uSize bandCount);

        /// @brief Appends samples to the ones the next Update analyzes. Only the newest WindowSize of them are kept.
        void AddSamples(std::span<const f32> samples);

        /// @brief Analyzes the newest WindowSize samples into the bands. A band rises to its new level at once and falls gradually.
        /// If no samples arrived since the last call - paused, stopped - the bands only fall.
        void Update();

        /// @brief The band levels from the lowest frequency to the highest, each 0 to 1. Valid until SetBandCount.
        std::span<const f32> GetBands() const;

    private:
        /// @brief The frequency bins one band takes its level from, both inclusive.
        struct BandBins
        {
            uSize
                _FirstBin { 0 },
                _LastBin { 0 };
        };

        /// @brief Works out which bins belong to which band. Needs the sample rate, so it does nothing before Init.
        void computeBandBins();

        AVTXContext* _context { nullptr };
        av_tx_fn _transform { nullptr };
        u32 _sampleRate { 0 };
        f32 _amplitudeScale { 0.0f }; // 2 / the window's sum: turns a bin's magnitude into the amplitude of a sine at that frequency

        std::vector<f32>
            _window,          // Hann window, so a tone shows as one narrow peak instead of smearing over every bin
            _history,         // the newest WindowSize samples, as a ring
            _windowedSamples, // the transform's input
            _bands;           // the transform's output, one level per band, 0 to 1
        std::vector<AVComplexFloat> _bins; // the transform's output: WindowSize / 2 + 1 bins from 0 Hz to half the sample rate
        std::vector<BandBins> _bandBins;
        uSize
            _historyIndex { 0 },   // where the next sample goes - and so where the oldest one is
            _newSampleCount { 0 }; // samples added since the last Update
        std::chrono::steady_clock::time_point _lastUpdateTime {};
        bool _hasUpdated { false };
        static inline constexpr f32
            _lowestFrequency { 30.0f },
            _highestFrequency { 20000.0f },
            _decibelFloor { -80.0f },
            _minimumAmplitude { 1.0e-6f },
            _fallPerSecond { 1.5f };
        static inline constexpr uSize _defaultBandCount { 32 };
    };
}
