//The decoder helper. Reads previews and thumbnails for IsolatedDecoder in a process of its own, so a file that crashes a
//decoder or takes it over (never reaches the program showing them). Requests come on standard input and replies go to
//standard output, each a 32-bit little-endian length and a MediaPreviewProtocol message. It ends when its input does
#include "MediaPreview.hpp"
#include "MediaPreviewProtocol.hpp"
#include <array>
#include <cstdio>
#include <optional>
#include <span>
#include <vector>

#if defined(TARGET_PLATFORM_WINDOWS)
#include <fcntl.h>
#include <io.h>
#endif

namespace
{
    bool readExactly(std::span<u8> data)
    {
        return data.size() == std::fread(data.data(), 1, data.size(), stdin);
    }

    bool writeMessage(const std::vector<u8>& message)
    {
        const u32 length { static_cast<u32>(message.size()) };
        const std::array<u8, 4> lengthBytes { static_cast<u8>(length), static_cast<u8>(length >> 8), static_cast<u8>(length >> 16), static_cast<u8>(length >> 24) };

        if(lengthBytes.size() != std::fwrite(lengthBytes.data(), 1, lengthBytes.size(), stdout))
            return false;

        if(message.size() != std::fwrite(message.data(), 1, message.size(), stdout))
            return false;

        return 0 == std::fflush(stdout);
    }
}

int main()
{
#if defined(TARGET_PLATFORM_WINDOWS)
    //Bytes as they are: text mode would turn every 10 into 13 10
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    while(true)
    {
        std::array<u8, 4> lengthBytes {};

        if(!readExactly(lengthBytes))
            return 0; //the program closed its end

        const u32 length { static_cast<u32>(lengthBytes[0]) | static_cast<u32>(lengthBytes[1]) << 8 | static_cast<u32>(lengthBytes[2]) << 16 | static_cast<u32>(lengthBytes[3]) << 24 };

        if(RetroFuturaGUI::MediaPreviewProtocol::MaxMessageSize < length)
            return 1;

        std::vector<u8> message(length);

        if(!readExactly(message))
            return 0;

        const std::optional<RetroFuturaGUI::MediaRequest> request { RetroFuturaGUI::MediaPreviewProtocol::DecodeRequest(message) };

        if(!request)
            return 1;

        RetroFuturaGUI::MediaPreviewResult result {};

        if(RetroFuturaGUI::MediaRequestType::Preview == request->_Type)
            result = RetroFuturaGUI::MediaPreview::ReadPreview(request->_Kind, request->_Path, request->_PictureSide);
        else if(RetroFuturaGUI::MediaRequestType::PreviewDetails == request->_Type)
            result = RetroFuturaGUI::MediaPreview::ReadPreviewDetails(request->_Kind, request->_Path, request->_PictureSide);
        else
            result._Picture = RetroFuturaGUI::MediaPreview::ReadThumbnail(request->_Kind, request->_Path, request->_PictureSide);

        if(!writeMessage(RetroFuturaGUI::MediaPreviewProtocol::EncodeResult(result)))
            return 0;
    }
}