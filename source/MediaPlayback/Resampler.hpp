#pragma once
#include "config.hpp"
#include "Frame.hpp"
#include <vector>

extern "C"
{
    #include <libswresample/swresample.h>
    #include <libavformat/avformat.h>
}

namespace RetroFuturaGUI
{
/// @brief Owns an Resampler for its whole lifetime.
    class Resampler
    {
    public:
        Resampler() = default;
        Resampler(const Resampler&) = delete;
        Resampler(Resampler&& other) = delete;
        ~Resampler();
        Resampler& operator=(const Resampler&) = delete;
        Resampler& operator=(Resampler&& other) = delete;

        bool Open(const AVStream* stream);
        void Close();
        bool IsOpen() const;

        i32 Convert(const Frame& frame);
        i32 Flush();
        const float* GetData() const;
        u32 GetChannels() const;
        u32 GetSampleRate() const;

    private:
        bool configure(const AVFrame* frame);
        bool inputMatches(const AVFrame* frame) const;
        i32 reserve(const i32 inSamples);
       
        SwrContext* _context { nullptr };
        AVChannelLayout _outLayout {};
        i32 _outRate { 0 };
        AVChannelLayout _inLayout {};
        AVSampleFormat _inFormat { AV_SAMPLE_FMT_NONE };
        i32 _inRate { 0 };
        std::vector<float> _buffer {};
    };    
}