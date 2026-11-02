#pragma once
#include "config.hpp"
#include <algorithm>
#include <array>
#include <string_view>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
}

//Set by CMake's RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY option; a build without it keeps to the royalty-free codecs
#ifndef RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY
#define RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY 1
#endif

namespace RetroFuturaGUI
{
    /// @brief Whether this build decodes royalty-free codecs only (CMake's RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY, on by
    ///        default). Off, every codec FFmpeg's build has decodes, and whoever ships the build takes on their licences.
    inline constexpr bool RoyaltyFreeCodecsOnly { 0 != RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY };

    /// @brief Codecs that are royalty-free or have their patents expired
    inline constexpr std::array FreeCodecs
    {
        // video
        AV_CODEC_ID_VP8, AV_CODEC_ID_VP9, AV_CODEC_ID_AV1,
        AV_CODEC_ID_MPEG2VIDEO, AV_CODEC_ID_MJPEG, AV_CODEC_ID_FFV1, AV_CODEC_ID_THEORA,
        // audio
        AV_CODEC_ID_FLAC, AV_CODEC_ID_VORBIS, AV_CODEC_ID_OPUS, AV_CODEC_ID_MP3,
        AV_CODEC_ID_ALAC, // Apple Lossless, Apache 2.0 since 2011
        AV_CODEC_ID_WAVPACK, // BSD-licensed, never patented
        AV_CODEC_ID_MP2, // MPEG-1 Layer II, patents expired
        AV_CODEC_ID_ADPCM_IMA_WAV, AV_CODEC_ID_ADPCM_IMA_QT, AV_CODEC_ID_ADPCM_MS, // decades old, patents expired
        // uncompressed PCM, which has no patents: every size, byte order and float, and A-law and µ-law
        AV_CODEC_ID_PCM_S8, AV_CODEC_ID_PCM_U8, AV_CODEC_ID_PCM_S8_PLANAR,
        AV_CODEC_ID_PCM_S16LE, AV_CODEC_ID_PCM_S16BE, AV_CODEC_ID_PCM_U16LE, AV_CODEC_ID_PCM_U16BE, AV_CODEC_ID_PCM_S16LE_PLANAR, AV_CODEC_ID_PCM_S16BE_PLANAR,
        AV_CODEC_ID_PCM_S24LE, AV_CODEC_ID_PCM_S24BE, AV_CODEC_ID_PCM_U24LE, AV_CODEC_ID_PCM_U24BE, AV_CODEC_ID_PCM_S24LE_PLANAR, AV_CODEC_ID_PCM_S24DAUD,
        AV_CODEC_ID_PCM_S32LE, AV_CODEC_ID_PCM_S32BE, AV_CODEC_ID_PCM_U32LE, AV_CODEC_ID_PCM_U32BE, AV_CODEC_ID_PCM_S32LE_PLANAR,
        AV_CODEC_ID_PCM_S64LE, AV_CODEC_ID_PCM_S64BE,
        AV_CODEC_ID_PCM_F16LE, AV_CODEC_ID_PCM_F24LE, AV_CODEC_ID_PCM_F32LE, AV_CODEC_ID_PCM_F32BE, AV_CODEC_ID_PCM_F64LE, AV_CODEC_ID_PCM_F64BE,
        AV_CODEC_ID_PCM_DVD, AV_CODEC_ID_PCM_BLURAY,
        AV_CODEC_ID_PCM_ALAW, AV_CODEC_ID_PCM_MULAW
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

    /// @brief Returns whether this build decodes a codec: a royalty-free one, or any while RoyaltyFreeCodecsOnly is off.
    constexpr bool IsCodecAllowed(const AVCodecID codecid)
    {
        return !RoyaltyFreeCodecsOnly || IsCodecFree(codecid);
    }

    /// @brief Returns whether a codec is known to need a patent licence to decode.
    constexpr bool IsCodecLicenceEncumbered(const AVCodecID codecid)
    {
        return std::ranges::find(NonCommercialUseCodecs, codecid) != NonCommercialUseCodecs.end();
    }
}