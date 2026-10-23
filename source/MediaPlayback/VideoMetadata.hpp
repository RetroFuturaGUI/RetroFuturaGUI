#pragma once
#include "config.hpp"
#include <optional>
#include <string>
#include <string_view>

namespace RetroFuturaGUI
{
    /// @brief What a video file says about itself, read without decoding a picture.
    struct VideoMetadata
    {
        std::string
            _Container, // FFmpeg's demuxer name
            _VideoCodec,
            _AudioCodec, // empty without sound
            _Title;
        f64
            _Seconds { 0.0 },
            _FramesPerSecond { 0.0 }; // the average rate, 0 when the container doesn't say
        u32
            _Width { 0 },
            _Height { 0 },
            _SampleRate { 0 },
            _Channels { 0 };
        i64 _BitRate { 0 };
        bool
            _VideoCodecIsFree { false }, // whether the picture may be decoded in a commercial product, see IsCodecFree
            _AudioCodecIsFree { false },
            _HasAudio { false };
    };

    /// @brief Reads what a video file says about itself. A picture attached as cover art, as songs carry, isn't a video.
    /// @param path UTF-8.
    /// @return Nothing when the file can't be opened or holds no video stream. Files whose codec isn't free read as well -
    ///         check _VideoCodecIsFree before decoding.
    std::optional<VideoMetadata> ReadVideoMetadata(std::string_view path);
}