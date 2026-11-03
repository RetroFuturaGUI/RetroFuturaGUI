//Everything the decoder helper does to a file: MediaPreview's preview or thumbnail of an image (stb_image, lunasvg), a song
//(the metadata readers, FFmpeg's cover decoders) or a video (FFmpeg's demuxers and royalty-free decoders, the frame scaler).
//The first byte picks the kind, the request and the extension.
#include "FuzzSetup.hpp"
#include "FuzzInput.hpp"
#include "MediaPreview.hpp"
#include <array>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    if(0 == size)
        return 0;

    struct Choice
    {
        RetroFuturaGUI::MediaKind _Kind;
        const char* _Extension;
    };

    constexpr std::array<Choice, 12> choices
    {{
        { ._Kind = RetroFuturaGUI::MediaKind::Image, ._Extension = ".png" },
        { ._Kind = RetroFuturaGUI::MediaKind::Image, ._Extension = ".jpg" },
        { ._Kind = RetroFuturaGUI::MediaKind::Image, ._Extension = ".gif" },
        { ._Kind = RetroFuturaGUI::MediaKind::Image, ._Extension = ".bmp" },
        { ._Kind = RetroFuturaGUI::MediaKind::Image, ._Extension = ".svg" },
        { ._Kind = RetroFuturaGUI::MediaKind::Audio, ._Extension = ".mp3" },
        { ._Kind = RetroFuturaGUI::MediaKind::Audio, ._Extension = ".flac" },
        { ._Kind = RetroFuturaGUI::MediaKind::Audio, ._Extension = ".ogg" },
        { ._Kind = RetroFuturaGUI::MediaKind::Video, ._Extension = ".webm" },
        { ._Kind = RetroFuturaGUI::MediaKind::Video, ._Extension = ".mkv" },
        { ._Kind = RetroFuturaGUI::MediaKind::Video, ._Extension = ".mp4" },
        { ._Kind = RetroFuturaGUI::MediaKind::Video, ._Extension = ".mpg" }
    }};
    const Choice& choice { choices[data[0] % choices.size()] };
    const bool isThumbnail { 0 != (data[0] & 0x80) };
    const RetroFuturaGUIFuzz::InputFile file { data + 1, size - 1, choice._Extension };

    if(isThumbnail)
        RetroFuturaGUI::MediaPreview::ReadThumbnail(choice._Kind, file.GetPath(), 64);
    else
        RetroFuturaGUI::MediaPreview::ReadPreview(choice._Kind, file.GetPath(), 128);

    return 0;
}