#pragma once
#include "config.hpp"
#include "Texture.hpp"
#include <optional>
#include <string_view>

namespace RetroFuturaGUI
{
    /// @brief Decodes the picture a video shown at a moment as RGBA pixels. GL is not touched so a worker thread can make a thumbnail of it.
    /// @param path UTF-8.
    /// @param milliseconds Frame from the start of the video. Past its end, the last frame is taken.
    /// @param maxSide When above 0, the picture is scaled down to fit inside a square that size.
    /// @return Nothing when the file can't be opened, holds no video, or its codec isn't free or fails to decode.
    std::optional<DecodedImage> DecodeVideoFrame(std::string_view path, const i64 milliseconds, const i32 maxSide, const bool flipVertically);
}