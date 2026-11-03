//The hand-written header parsers that take bytes: the first byte picks ReadHcaHeader or ReadMatroskaHeader
#include "FuzzSetup.hpp"
#include "AudioMetadataReader.hpp"
#include <cstdint>
#include <span>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    if(0 == size)
        return 0;

    const std::span<const u8> header { data + 1, size - 1 };

    if(0 == (data[0] & 1))
        RetroFuturaGUI::AudioMetadataReader::ReadHcaHeader(header);
    else
        RetroFuturaGUI::AudioMetadataReader::ReadMatroskaHeader(header);

    return 0;
}