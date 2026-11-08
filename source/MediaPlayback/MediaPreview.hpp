#pragma once
#include "config.hpp"
#include "Texture.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The kinds of file MediaPreview reads
    enum class MediaKind : u32
    {
        Image,
        Audio,
        Video
    };

    /// @brief What a file's preview shows: the details only this kind of file has, a picture, or a message in its place
    struct MediaPreviewResult
    {
        std::vector<std::pair<std::string, std::string>> _Details {};
        std::optional<DecodedImage> _Picture {};
        std::string _Message {}; //no cover art. A codec that isn't royalty-free is a file that can't be read if non-royality-free codecs are rejected
        bool _CanShowFrame { false }; //a video whose frame was decoded here, so the program may open it to play it
    };

    /// @brief Reads what the file explorer's preview and thumbnails show of an image (a song or a video without GL or widgets) The
    ///        work IsolatedDecoder has the decoder helper do in a process of its own, or does here when the helper is missing
    class MediaPreview
    {
    public:
        /// @brief The preview: an image's picture and size. A song's tags, length, format and cover art. A video's length, size,
        ///        rate and codecs (a WebM or Matroska file with sound only is read as a song). A video's frame from 10% in is
        ///        decoded here as its picture (_CanShowFrame), so the program shows it without decoding the file, and a file that
        ///        crashes the decoder never reaches the program
        /// @param path UTF-8
        static MediaPreviewResult ReadPreview(const MediaKind kind, std::string_view path, const i32 pictureSide);

        /// @brief The thumbnail: an image scaled down, a song's cover art, a video's frame from 10% in for royalty-free (if non-royality-free is rejected) codecs
        static std::optional<DecodedImage> ReadThumbnail(const MediaKind kind, std::string_view path, const i32 pictureSide);

        /// @brief "MM:SS", or "HH:MM:SS" past an hour.
        static std::string FormatDuration(const f64 seconds);

        /// @brief "H.264" for FFmpeg's "h264" (Names of non-royality-free codes are still known tho)
        static std::string CodecName(std::string_view ffmpegName);

    private:
        MediaPreview() = delete;

        /// @brief A song's details and cover art. False with nothing added when the file can't be read as audio
        static bool readAudio(std::string_view path, const i32 pictureSide, MediaPreviewResult& result);

        /// @brief A song's cover art scaled down. None without one
        static std::optional<DecodedImage> readCoverArt(std::string_view path, const i32 pictureSide);

        /// @brief One row of details. An empty value adds none
        static void addDetail(MediaPreviewResult& result, std::string_view key, std::string value);

    };
}