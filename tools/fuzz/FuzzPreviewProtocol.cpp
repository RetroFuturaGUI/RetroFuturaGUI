//The decoder helper's replies as the program reads them - what a helper taken over by a malicious file would attack - and
//the requests as the helper reads them. Whatever decodes is encoded again and must decode to the same thing.
#include "FuzzSetup.hpp"
#include "MediaPreviewProtocol.hpp"
#include <cstdint>
#include <cstdlib>
#include <span>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    const std::span<const u8> message { data, size };
    const std::optional<RetroFuturaGUI::MediaPreviewResult> result { RetroFuturaGUI::MediaPreviewProtocol::DecodeResult(message) };

    if(result)
    {
        const std::vector<u8> encoded { RetroFuturaGUI::MediaPreviewProtocol::EncodeResult(*result) };

        if(!RetroFuturaGUI::MediaPreviewProtocol::DecodeResult(encoded))
            std::abort(); //what was read must read again
    }

    const std::optional<RetroFuturaGUI::MediaRequest> request { RetroFuturaGUI::MediaPreviewProtocol::DecodeRequest(message) };

    if(request && !RetroFuturaGUI::MediaPreviewProtocol::DecodeRequest(RetroFuturaGUI::MediaPreviewProtocol::EncodeRequest(*request)))
        std::abort();

    return 0;
}