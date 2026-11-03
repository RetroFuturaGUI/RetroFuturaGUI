//AudioMetadataReader::Read on any file: FFmpeg's demuxers and every hand-written reader behind them. The WAV smpl chunk,
//the MP3 Xing/LAME header, ADX and HCA headers, the Vorbis and Opus headers, AIFF chunks, the Matroska header, cover art -
//and libopenmpt for modules. The first byte picks the file's extension, which some of FFmpeg's probes weigh.
#include "FuzzSetup.hpp"
#include "AudioMetadataReader.hpp"
#include "FuzzInput.hpp"
#include <array>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    if(0 == size)
        return 0;

    constexpr std::array extensions { ".wav", ".mp3", ".flac", ".ogg", ".opus", ".m4a", ".adx", ".hca", ".aiff", ".mka", ".webm", ".mod", ".xm", ".it", ".s3m", ".bin" };
    const RetroFuturaGUIFuzz::InputFile file { data + 1, size - 1, extensions[data[0] % extensions.size()] };
    const std::optional<RetroFuturaGUI::AudioMetadata> metadata { RetroFuturaGUI::AudioMetadataReader::Read(file.GetPath()) };
    return 0;
}