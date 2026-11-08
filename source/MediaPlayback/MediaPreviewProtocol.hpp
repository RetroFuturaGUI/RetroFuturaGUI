#pragma once
#include "MediaPreview.hpp"
#include "config.hpp"
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief What the decoder helper is asked for
    enum class MediaRequestType : u32
    {
        Preview,
        Thumbnail,
        PreviewDetails //the preview without a video's picture
    };

    /// @brief One request to the decoder helper
    struct MediaRequest
    {
        MediaRequestType _Type { MediaRequestType::Preview };
        MediaKind _Kind { MediaKind::Image };
        i32 _PictureSide { 0 };
        std::string _Path {}; //UTF-8
    };

    /// @brief The messages between IsolatedDecoder and the decoder helper (little endian): each a 32-bit length, then the message. 
    // /       A reply is read as untrusted. A helper taken over by a malicious file could send anything, so every length is checked against what's left and against limits before anything is allocated.
    class MediaPreviewProtocol
    {
    public:
        /// @brief The longest message either side accepts: a picture of _maxPictureSide squared and its details fit
        static constexpr u32 MaxMessageSize { 80u * 1024 * 1024 };

        static std::vector<u8> EncodeRequest(const MediaRequest& request);
        static std::optional<MediaRequest> DecodeRequest(std::span<const u8> message);

        /// @brief A thumbnail travels as a result with a picture only.
        static std::vector<u8> EncodeResult(const MediaPreviewResult& result);
        static std::optional<MediaPreviewResult> DecodeResult(std::span<const u8> message);

    private:
        MediaPreviewProtocol() = delete;

        static void writeU32(std::vector<u8>& message, const u32 value);
        static void writeString(std::vector<u8>& message, std::string_view text);

        /// @brief Reads from offset which moves past what was read. Nothing when the message ends first or a string is longer than _maxStringSize
        static std::optional<u32> readU32(std::span<const u8> message, uSize& offset);
        static std::optional<std::string> readString(std::span<const u8> message, uSize& offset);

        static constexpr u32
            _maxStringSize { 64 * 1024 },
            _maxDetailCount { 256 },
            _maxPictureSide { 4096 };
    };
}