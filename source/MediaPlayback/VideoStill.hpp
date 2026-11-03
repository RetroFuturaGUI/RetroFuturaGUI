#pragma once
#include "AudioMetadata.hpp"
#include "config.hpp"
#include "Texture.hpp"
#include <optional>
#include <span>
#include <string_view>

struct AVFrame;

namespace RetroFuturaGUI
{
    /// @brief Decodes the picture a video shown at a moment as RGBA pixels. GL is not touched so a worker thread can make a thumbnail of it.
    /// @param path UTF-8.
    /// @param milliseconds Frame from the start of the video. Past its end, the last frame is taken.
    /// @param maxSide When above 0, the picture is scaled down to fit inside a square that size.
    /// @return Nothing when the file can't be opened, holds no video, or its codec isn't free or fails to decode.
    std::optional<DecodedImage> DecodeVideoFrame(std::string_view path, const i64 milliseconds, const i32 maxSide, const bool flipVertically);

    /// @brief Decodes a file's cover art (AudioMetadata's _CoverArt and _CoverArtCodec) as RGBA pixels with FFmpeg's decoders:
    ///        JPEG, PNG, BMP, GIF (its first frame), WebP and TIFF. GL is not touched.
    /// @param maxSide When above 0, the picture is scaled down to fit inside a square that size.
    /// @return Nothing without a picture, for CoverArtCodec::Other, or when it fails to decode.
    std::optional<DecodedImage> DecodeCoverArt(std::span<const u8> picture, const CoverArtCodec codec, const i32 maxSide, const bool flipVertically);

    /// @brief A decoded picture as RGBA pixels at its display shape (wide pixels widened by pixelAspect) scaled down to fit
    ///        maxSide when above 0, in its own colors (the color matrix and range it carries). What both decoders above end with.
    std::optional<DecodedImage> ConvertPictureToImage(const AVFrame* picture, const f64 pixelAspect, const i32 maxSide, const bool flipVertically);
}