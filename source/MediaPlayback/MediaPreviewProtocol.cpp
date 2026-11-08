#include "MediaPreviewProtocol.hpp"

std::vector<u8> RetroFuturaGUI::MediaPreviewProtocol::EncodeRequest(const MediaRequest& request)
{
    std::vector<u8> message {};
    writeU32(message, static_cast<u32>(request._Type));
    writeU32(message, static_cast<u32>(request._Kind));
    writeU32(message, static_cast<u32>(request._PictureSide));
    writeString(message, request._Path);
    return message;
}

std::optional<RetroFuturaGUI::MediaRequest> RetroFuturaGUI::MediaPreviewProtocol::DecodeRequest(std::span<const u8> message)
{
    uSize offset { 0 };
    const std::optional<u32>
        type { readU32(message, offset) },
        kind { readU32(message, offset) },
        side { readU32(message, offset) };
    std::optional<std::string> path { readString(message, offset) };

    if(!type || !kind || !side || !path)
        return std::nullopt;

    if(static_cast<u32>(MediaRequestType::PreviewDetails) < *type || static_cast<u32>(MediaKind::Video) < *kind || _maxPictureSide < *side)
        return std::nullopt;

    return MediaRequest { ._Type = static_cast<MediaRequestType>(*type), ._Kind = static_cast<MediaKind>(*kind), ._PictureSide = static_cast<i32>(*side), ._Path = std::move(*path) };
}

std::vector<u8> RetroFuturaGUI::MediaPreviewProtocol::EncodeResult(const MediaPreviewResult& result)
{
    std::vector<u8> message {};
    writeU32(message, static_cast<u32>(result._Details.size()));

    for(const auto& [key, value] : result._Details)
    {
        writeString(message, key);
        writeString(message, value);
    }

    writeString(message, result._Message);
    writeU32(message, result._CanShowFrame ? 1 : 0);
    writeU32(message, result._Picture ? 1 : 0);

    if(!result._Picture)
        return message;

    const DecodedImage& picture { *result._Picture };
    writeU32(message, static_cast<u32>(picture._Resolution.x));
    writeU32(message, static_cast<u32>(picture._Resolution.y));
    writeU32(message, static_cast<u32>(picture._SourceResolution.x));
    writeU32(message, static_cast<u32>(picture._SourceResolution.y));
    writeU32(message, picture._IsFlipped ? 1 : 0);
    message.insert(message.end(), picture._Pixels.begin(), picture._Pixels.end());
    return message;
}

std::optional<RetroFuturaGUI::MediaPreviewResult> RetroFuturaGUI::MediaPreviewProtocol::DecodeResult(std::span<const u8> message)
{
    uSize offset { 0 };
    const std::optional<u32> detailCount { readU32(message, offset) };

    if(!detailCount || _maxDetailCount < *detailCount)
        return std::nullopt;

    MediaPreviewResult result {};

    for(u32 detail = 0; detail < *detailCount; ++detail)
    {
        std::optional<std::string>
            key { readString(message, offset) },
            value { readString(message, offset) };

        if(!key || !value)
            return std::nullopt;

        result._Details.emplace_back(std::move(*key), std::move(*value));
    }

    std::optional<std::string> text { readString(message, offset) };
    const std::optional<u32>
        canShowFrame { readU32(message, offset) },
        hasPicture { readU32(message, offset) };

    if(!text || !canShowFrame || !hasPicture)
        return std::nullopt;

    result._Message = std::move(*text);
    result._CanShowFrame = 0 != *canShowFrame;

    if(0 == *hasPicture)
        return offset == message.size() ? std::optional<MediaPreviewResult>(std::move(result)) : std::nullopt;

    const std::optional<u32>
        width { readU32(message, offset) },
        height { readU32(message, offset) },
        sourceWidth { readU32(message, offset) },
        sourceHeight { readU32(message, offset) },
        isFlipped { readU32(message, offset) };

    if(!width || !height || !sourceWidth || !sourceHeight || !isFlipped)
        return std::nullopt;

    if(0 == *width || 0 == *height || _maxPictureSide < *width || _maxPictureSide < *height || INT32_MAX < *sourceWidth || INT32_MAX < *sourceHeight)
        return std::nullopt;

    // Exactly the pixels the size says and nothing after them
    const uSize pixelBytes { static_cast<uSize>(*width) * *height * 4 };

    if(pixelBytes != message.size() - offset)
        return std::nullopt;

    result._Picture = DecodedImage
    {
        ._Pixels = std::vector<u8>(message.begin() + static_cast<std::ptrdiff_t>(offset), message.end()),
        ._Resolution = glm::i32vec2(static_cast<i32>(*width), static_cast<i32>(*height)),
        ._SourceResolution = glm::i32vec2(static_cast<i32>(*sourceWidth), static_cast<i32>(*sourceHeight)),
        ._IsFlipped = 0 != *isFlipped
    };
    return result;
}

void RetroFuturaGUI::MediaPreviewProtocol::writeU32(std::vector<u8>& message, const u32 value)
{
    for(u32 shift = 0; shift < 32; shift += 8)
        message.push_back(static_cast<u8>(value >> shift));
}

void RetroFuturaGUI::MediaPreviewProtocol::writeString(std::vector<u8>& message, std::string_view text)
{
    const std::string_view kept { text.substr(0, _maxStringSize) };
    writeU32(message, static_cast<u32>(kept.size()));
    message.insert(message.end(), kept.begin(), kept.end());
}

std::optional<u32> RetroFuturaGUI::MediaPreviewProtocol::readU32(std::span<const u8> message, uSize& offset)
{
    if(4 > message.size() - std::min(offset, message.size()))
        return std::nullopt;

    const u32 value { static_cast<u32>(message[offset]) | static_cast<u32>(message[offset + 1]) << 8 | static_cast<u32>(message[offset + 2]) << 16 | static_cast<u32>(message[offset + 3]) << 24 };
    offset += 4;
    return value;
}

std::optional<std::string> RetroFuturaGUI::MediaPreviewProtocol::readString(std::span<const u8> message, uSize& offset)
{
    const std::optional<u32> length { readU32(message, offset) };

    if(!length || _maxStringSize < *length || *length > message.size() - offset)
        return std::nullopt;

    std::string text(reinterpret_cast<const char*>(message.data() + offset), *length);
    offset += *length;
    return text;
}