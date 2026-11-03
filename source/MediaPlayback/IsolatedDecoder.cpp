#include "IsolatedDecoder.hpp"
#include "Paths.hpp"
#include <array>

RetroFuturaGUI::MediaPreviewResult RetroFuturaGUI::IsolatedDecoder::ReadPreview(const MediaKind kind, std::string_view path, const i32 pictureSide)
{
    if(!IsHelperAvailable())
        return MediaPreview::ReadPreview(kind, path, pictureSide);

    std::optional<MediaPreviewResult> result { exchange(MediaRequest { ._Type = MediaRequestType::Preview, ._Kind = kind, ._PictureSide = pictureSide, ._Path = std::string(path) }) };

    // Not tried again here... A file that crashes the helper would crash this process!
    if(!result)
        return MediaPreviewResult { ._Details = {}, ._Picture = {}, ._Message = "Can't read this file", ._CanShowFrame = false };

    return std::move(*result);
}

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::IsolatedDecoder::ReadThumbnail(const MediaKind kind, std::string_view path, const i32 pictureSide)
{
    if(!IsHelperAvailable())
        return MediaPreview::ReadThumbnail(kind, path, pictureSide);

    std::optional<MediaPreviewResult> result { exchange(MediaRequest { ._Type = MediaRequestType::Thumbnail, ._Kind = kind, ._PictureSide = pictureSide, ._Path = std::string(path) }) };

    if(!result)
        return std::nullopt; //the file keeps its icon

    return std::move(result->_Picture);
}

std::filesystem::path RetroFuturaGUI::IsolatedDecoder::GetHelperPath()
{
#if defined(TARGET_PLATFORM_WINDOWS)
    return PlatformBridge::Paths::GetModuleFolder() / "RetroFuturaGUIDecoder.exe";
#else
    return PlatformBridge::Paths::GetModuleFolder() / "RetroFuturaGUIDecoder";
#endif
}

bool RetroFuturaGUI::IsolatedDecoder::IsHelperAvailable()
{
    static const bool isAvailable { std::filesystem::is_regular_file(GetHelperPath()) }; //shipped or not, it doesn't change while running
    return isAvailable;
}

std::optional<RetroFuturaGUI::MediaPreviewResult> RetroFuturaGUI::IsolatedDecoder::exchange(const MediaRequest& request)
{
    if(!_helper.IsRunning() && !_helper.Start(GetHelperPath(), PlatformBridge::ChildProcess::ChildProcessFlags::Sandboxed))
        return std::nullopt;

    const std::vector<u8> message { MediaPreviewProtocol::EncodeRequest(request) };
    const u32 length { static_cast<u32>(message.size()) };
    const std::array<u8, 4> lengthBytes { static_cast<u8>(length), static_cast<u8>(length >> 8), static_cast<u8>(length >> 16), static_cast<u8>(length >> 24) };

    if(!_helper.Write(lengthBytes) || !_helper.Write(message))
    {
        _helper.Stop();
        return std::nullopt;
    }

    std::array<u8, 4> replyLengthBytes {};

    if(!_helper.Read(replyLengthBytes, _replyPatience))
    {
        _helper.Stop();
        return std::nullopt;
    }

    const u32 replyLength { static_cast<u32>(replyLengthBytes[0]) | static_cast<u32>(replyLengthBytes[1]) << 8 | static_cast<u32>(replyLengthBytes[2]) << 16 | static_cast<u32>(replyLengthBytes[3]) << 24 };

    if(MediaPreviewProtocol::MaxMessageSize < replyLength)
    {
        _helper.Stop(); //taken over, or broken
        return std::nullopt;
    }

    std::vector<u8> reply(replyLength);

    if(!_helper.Read(reply, _replyPatience))
    {
        _helper.Stop();
        return std::nullopt;
    }

    std::optional<MediaPreviewResult> result { MediaPreviewProtocol::DecodeResult(reply) };

    if(!result)
        _helper.Stop();

    return result;
}