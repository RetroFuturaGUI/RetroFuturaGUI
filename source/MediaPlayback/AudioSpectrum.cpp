#include "AudioSpectrum.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

RetroFuturaGUI::AudioSpectrum::AudioSpectrum()
    : _window(WindowSize), _history(WindowSize, 0.0f), _windowedSamples(WindowSize), _bands(_defaultBandCount, 0.0f), _bins(WindowSize / 2 + 1)
{
    f32 windowSum { 0.0f };

    for(uSize sampleIndex = 0; sampleIndex < WindowSize; ++sampleIndex)
    {
        _window[sampleIndex] = 0.5f - 0.5f * std::cos(2.0f * std::numbers::pi_v<f32> * static_cast<f32>(sampleIndex) / static_cast<f32>(WindowSize));
        windowSum += _window[sampleIndex];
    }

    _amplitudeScale = 2.0f / windowSum;
}

RetroFuturaGUI::AudioSpectrum::~AudioSpectrum()
{
    Uninit();
}

bool RetroFuturaGUI::AudioSpectrum::Init(const u32 sampleRate)
{
    if(0 == sampleRate)
        return false;

    Uninit();

    const f32 scale { 1.0f };

    if(0 > av_tx_init(&_context, &_transform, AV_TX_FLOAT_RDFT, 0, static_cast<i32>(WindowSize), &scale, AV_TX_UNALIGNED))
        return false;

    _sampleRate = sampleRate;
    computeBandBins();
    return true;
}

void RetroFuturaGUI::AudioSpectrum::Uninit()
{
    av_tx_uninit(&_context);
    _transform = nullptr;
    _sampleRate = 0;
    _bandBins.clear();
    std::fill(_history.begin(), _history.end(), 0.0f);
    std::fill(_bands.begin(), _bands.end(), 0.0f);
    _historyIndex = 0;
    _newSampleCount = 0;
    _hasUpdated = false;
}

void RetroFuturaGUI::AudioSpectrum::SetBandCount(const uSize bandCount)
{
    _bands.assign(bandCount, 0.0f);
    computeBandBins();
}

void RetroFuturaGUI::AudioSpectrum::AddSamples(std::span<const f32> samples)
{
    if(samples.size() > WindowSize)
        samples = samples.last(WindowSize);

    for(const f32 sample : samples)
    {
        _history[_historyIndex] = sample;
        _historyIndex = (_historyIndex + 1) % WindowSize;
    }

    _newSampleCount += samples.size();
}

void RetroFuturaGUI::AudioSpectrum::Update()
{
    if(!_context)
        return;

    if(!_transform)
        return;

    const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };
    const f32 elapsedSeconds { _hasUpdated ? std::chrono::duration<f32>(now - _lastUpdateTime).count() : 0.0f };
    const f32 fall { _fallPerSecond * elapsedSeconds };
    _lastUpdateTime = now;
    _hasUpdated = true;

    if(0 == _newSampleCount)
    {
        for(f32& band : _bands)
            band = fall < band ? band - fall : 0.0f;

        return;
    }

    _newSampleCount = 0;

    for(uSize sampleIndex = 0; sampleIndex < WindowSize; ++sampleIndex)
        _windowedSamples[sampleIndex] = _history[(_historyIndex + sampleIndex) % WindowSize] * _window[sampleIndex];

    _transform(_context, _bins.data(), _windowedSamples.data(), sizeof(f32));

    for(uSize bandIndex = 0; bandIndex < _bands.size(); ++bandIndex)
    {
        const BandBins& bandBins { _bandBins[bandIndex] };
        f32 peakSquared { 0.0f };

        for(uSize binIndex = bandBins._FirstBin; binIndex <= bandBins._LastBin; ++binIndex)
        {
            const AVComplexFloat& bin { _bins[binIndex] };
            const f32 magnitudeSquared { bin.re * bin.re + bin.im * bin.im };

            if(magnitudeSquared > peakSquared)
                peakSquared = magnitudeSquared;
        }

        const f32
            amplitude { std::sqrt(peakSquared) * _amplitudeScale },
            decibels { 20.0f * std::log10(amplitude > _minimumAmplitude ? amplitude : _minimumAmplitude) },
            level { std::clamp((decibels - _decibelFloor) / -_decibelFloor, 0.0f, 1.0f) },
            fallen { fall < _bands[bandIndex] ? _bands[bandIndex] - fall : 0.0f };

        _bands[bandIndex] = level > fallen ? level : fallen;
    }
}

std::span<const f32> RetroFuturaGUI::AudioSpectrum::GetBands() const
{
    return _bands;
}

void RetroFuturaGUI::AudioSpectrum::computeBandBins()
{
    _bandBins.clear();

    if(0 == _sampleRate)
        return;

    if(_bands.empty())
        return;

    const uSize highestBin { WindowSize / 2 };
    const f32
        binWidth { static_cast<f32>(_sampleRate) / static_cast<f32>(WindowSize) },
        halfSampleRate { static_cast<f32>(_sampleRate) * 0.5f },
        highestFrequency { _highestFrequency < halfSampleRate ? _highestFrequency : halfSampleRate },
        bandRatio { std::pow(highestFrequency / _lowestFrequency, 1.0f / static_cast<f32>(_bands.size())) };

    f32 lowerEdge { _lowestFrequency };

    for(uSize bandIndex = 0; bandIndex < _bands.size(); ++bandIndex)
    {
        const f32 upperEdge { lowerEdge * bandRatio };

        uSize
            firstBin { static_cast<uSize>(std::ceil(lowerEdge / binWidth)) },
            lastBin { static_cast<uSize>(std::ceil(upperEdge / binWidth)) };

        if(firstBin < lastBin)
            --lastBin;
        else
        {
            const uSize nearestBin { static_cast<uSize>(std::round(std::sqrt(lowerEdge * upperEdge) / binWidth)) };
            firstBin = nearestBin;
            lastBin = nearestBin;
        }

        firstBin = std::clamp<uSize>(firstBin, 1, highestBin);
        lastBin = std::clamp<uSize>(lastBin, firstBin, highestBin);
        _bandBins.push_back(BandBins { ._FirstBin = firstBin, ._LastBin = lastBin });
        lowerEdge = upperEdge;
    }
}
