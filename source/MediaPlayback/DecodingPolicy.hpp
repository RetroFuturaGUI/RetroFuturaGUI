#pragma once
#include "config.hpp"
#include <algorithm>
#include <array>
#include <string_view>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    //#include <libswresample/swresample.h>
    #include <libavutil/opt.h>
}

namespace RetroFuturaGUI
{
    /// @brief Codecs that are royalty-free or have their patents expired
    inline constexpr std::array FreeCodecs
    {
        // video
        AV_CODEC_ID_VP8, AV_CODEC_ID_VP9, AV_CODEC_ID_AV1,
        AV_CODEC_ID_MPEG2VIDEO, AV_CODEC_ID_MJPEG, AV_CODEC_ID_FFV1, AV_CODEC_ID_THEORA,
        // audio
        AV_CODEC_ID_FLAC, AV_CODEC_ID_VORBIS, AV_CODEC_ID_OPUS,
        AV_CODEC_ID_MP3, AV_CODEC_ID_PCM_S16LE
    };

    /// @brief Codecs whose patent pools require a paid licence to distribute a decoder.
    /// Listed so a refusal can say which codec it refused and why, rather than just
    /// reporting an unsupported file.
    inline constexpr std::array NonCommercialUseCodecs
    {
        // video
        AV_CODEC_ID_H264, AV_CODEC_ID_HEVC, AV_CODEC_ID_VVC,
        AV_CODEC_ID_VC1, AV_CODEC_ID_MPEG4,
        // audio
        AV_CODEC_ID_AAC, AV_CODEC_ID_AAC_LATM
    };

    /// @brief Returns whether a codec is free to decode in a commercial product.
    constexpr bool IsCodecFree(const AVCodecID codecid)
    {
        return std::ranges::find(FreeCodecs, codecid) != FreeCodecs.end();
    }

    /// @brief Returns whether a codec is known to need a patent licence to decode.
    constexpr bool IsCodecLicenceEncumbered(const AVCodecID codecid)
    {
        return std::ranges::find(NonCommercialUseCodecs, codecid) != NonCommercialUseCodecs.end();
    }
}