#include "AudioMetadataReader.hpp"
#include "MediaSource.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <initializer_list>

extern "C"
{
    #include <libavutil/avstring.h>
    #include <libavutil/intreadwrite.h>
}

std::string RetroFuturaGUI::AudioMetadataReader::toString(const char* text)
{
    if(!text)
        return {};

    return text;
}

std::string RetroFuturaGUI::AudioMetadataReader::toHex(const u8* bytes, const uSize count)
{
    if(!bytes)
        return {};

    std::string hex;

    for(uSize i = 0; i < count; ++i)
        hex += std::format("{:02x}", bytes[i]);

    return hex;
}

std::optional<u64> RetroFuturaGUI::AudioMetadataReader::parseUnsigned(const std::string& text)
{
    u64 value { 0 };
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);

    if(error != std::errc())
        return std::nullopt;

    return value;
}

std::vector<u64> RetroFuturaGUI::AudioMetadataReader::parseHexFields(const std::string& text)
{
    std::vector<u64> fields;
    const char* position = text.data();
    const char* end = text.data() + text.size();

    while(position < end)
    {
        while(position < end && *position == ' ')
            ++position;

        if(position == end)
            break;

        u64 value { 0 };
        const auto [next, error] = std::from_chars(position, end, value, 16);

        if(error != std::errc())
            break;

        fields.push_back(value);
        position = next;
    }

    return fields;
}

f64 RetroFuturaGUI::AudioMetadataReader::toSeconds(const u64 samples, const u32 sampleRate)
{
    if(0 == sampleRate)
        return 0.0;

    return static_cast<f64>(samples) / sampleRate;
}

std::string RetroFuturaGUI::AudioMetadataReader::describeLayout(const AVChannelLayout& layout)
{
    std::array<char, 128> buffer {};

    if(0 >= av_channel_layout_describe(&layout, buffer.data(), buffer.size()))
        return {};

    return buffer.data();
}

std::ifstream RetroFuturaGUI::AudioMetadataReader::openFile(std::string_view path)
{
    const std::u8string utf8(path.begin(), path.end());

    return std::ifstream(std::filesystem::path(utf8), std::ios::binary);
}

u64 RetroFuturaGUI::AudioMetadataReader::getFileSize(std::ifstream& file)
{
    file.clear();
    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();

    return 0 < size ? static_cast<u64>(size) : 0;
}

std::vector<u8> RetroFuturaGUI::AudioMetadataReader::readBytes(std::ifstream& file, const u64 offset, const u64 count)
{
    std::vector<u8> bytes(static_cast<uSize>(count));

    file.clear();
    file.seekg(static_cast<std::streamoff>(offset));
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(count));
    bytes.resize(static_cast<uSize>(std::max<std::streamsize>(file.gcount(), 0)));

    return bytes;
}

std::string RetroFuturaGUI::AudioMetadataReader::findTag(const AVDictionary* dictionary, const char* key)
{
    // av_dict_get matches keys case-insensitively, so "title" also finds a Vorbis comment's "TITLE"
    const AVDictionaryEntry* entry = av_dict_get(dictionary, key, nullptr, 0);

    if(!entry)
        return {};

    return toString(entry->value);
}

std::string RetroFuturaGUI::AudioMetadataReader::findTag(const AVDictionary* fileTags, const AVDictionary* streamTags, const std::initializer_list<const char*> keys)
{
    for(const char* key : keys)
    {
        std::string value = findTag(fileTags, key);

        if(value.empty())
            value = findTag(streamTags, key);

        if(!value.empty())
            return value;
    }

    return {};
}

void RetroFuturaGUI::AudioMetadataReader::appendTags(const AVDictionary* dictionary, std::vector<std::pair<std::string, std::string>>& tags)
{
    for(const AVDictionaryEntry* entry = av_dict_iterate(dictionary, nullptr); entry; entry = av_dict_iterate(dictionary, entry))
    {
        const bool alreadyListed = std::ranges::any_of(tags, [entry](const std::pair<std::string, std::string>& tag)
        {
            return 0 == av_strcasecmp(tag.first.c_str(), entry->key);
        });

        if(alreadyListed)
            continue;

        tags.emplace_back(toString(entry->key), toString(entry->value));
    }
}

std::optional<RetroFuturaGUI::AudioLoop> RetroFuturaGUI::AudioMetadataReader::readLoopTags(const AVDictionary* fileTags, const AVDictionary* streamTags)
{
    const std::optional<u64> start = parseUnsigned(findTag(fileTags, streamTags, { "LOOPSTART", "LOOP_START" }));

    if(!start)
        return std::nullopt;

    const std::optional<u64> length = parseUnsigned(findTag(fileTags, streamTags, { "LOOPLENGTH", "LOOP_LENGTH" }));

    if(length)
        return RetroFuturaGUI::AudioLoop { ._StartSample = *start, ._EndSample = *start + *length, ._Source = "LOOPSTART/LOOPLENGTH tags" };

    const std::optional<u64> end = parseUnsigned(findTag(fileTags, streamTags, { "LOOPEND", "LOOP_END" }));

    if(!end)
        return std::nullopt;

    return RetroFuturaGUI::AudioLoop { ._StartSample = *start, ._EndSample = *end, ._Source = "LOOPSTART/LOOPEND tags" };
}

std::vector<RetroFuturaGUI::AudioChapter> RetroFuturaGUI::AudioMetadataReader::readChapters(const AVFormatContext* context)
{
    std::vector<RetroFuturaGUI::AudioChapter> chapters;

    if(!context)
        return chapters;

    for(u32 i = 0; i < context->nb_chapters; ++i)
    {
        const AVChapter* chapter = context->chapters[i];

        if(!chapter)
            continue;

        const f64 timeBase = av_q2d(chapter->time_base);

        chapters.push_back({
            ._StartSeconds = static_cast<f64>(chapter->start) * timeBase,
            ._EndSeconds = chapter->end == AV_NOPTS_VALUE ? -1.0 : static_cast<f64>(chapter->end) * timeBase,
            ._Title = findTag(chapter->metadata, "title") });
    }

    return chapters;
}

uSize RetroFuturaGUI::AudioMetadataReader::findPictureStart(const u8* data, const uSize size)
{
    constexpr uSize searchLimit { 16 };
    constexpr std::array<u8, 3> jpegSignature { 0xFF, 0xD8, 0xFF };
    constexpr std::array<u8, 4> pngSignature { 0x89, 'P', 'N', 'G' };

    for(uSize start = 0; start < searchLimit && start < size; ++start)
    {
        const bool
            isJpeg { start + jpegSignature.size() <= size && std::equal(jpegSignature.begin(), jpegSignature.end(), data + start) },
            isPng { start + pngSignature.size() <= size && std::equal(pngSignature.begin(), pngSignature.end(), data + start) };

        if(isJpeg || isPng)
            return start;
    }

    return 0; // another format, kept as it is
}

void RetroFuturaGUI::AudioMetadataReader::readCoverArt(const AVFormatContext* context, RetroFuturaGUI::AudioMetadata& metadata)
{
    if(!context)
        return;

    for(u32 i = 0; i < context->nb_streams; ++i)
    {
        const AVStream* stream = context->streams[i];

        if(!stream)
            continue;

        if(!(stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
            continue;

        const AVCodecParameters* picture = stream->codecpar;

        if(!picture)
            continue;

        metadata._CoverArtCodec = toString(avcodec_get_name(picture->codec_id));
        metadata._CoverArtWidth = static_cast<u32>(std::max(picture->width, 0));
        metadata._CoverArtHeight = static_cast<u32>(std::max(picture->height, 0));

        if(!stream->attached_pic.data)
            return;

        const uSize
            size { static_cast<uSize>(std::max(stream->attached_pic.size, 0)) },
            start { findPictureStart(stream->attached_pic.data, size) };
        metadata._CoverArt.assign(stream->attached_pic.data + start, stream->attached_pic.data + size);
        return; // the first picture only - it's the front cover in practice
    }
}

bool RetroFuturaGUI::AudioMetadataReader::hasVideoStream(const AVFormatContext* context)
{
    if(!context)
        return false;

    for(u32 i = 0; i < context->nb_streams; ++i)
    {
        const AVStream* stream = context->streams[i];

        if(!stream)
            continue;

        if(!stream->codecpar)
            continue;

        if(stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
            continue;

        // cover art is a video stream as well, but a still one
        if(stream->disposition & AV_DISPOSITION_ATTACHED_PIC)
            continue;

        return true;
    }

    return false;
}

RetroFuturaGUI::AudioFileFormat RetroFuturaGUI::AudioMetadataReader::classify(const AVFormatContext* context, const AVCodecParameters* audio)
{
    using RetroFuturaGUI::AudioFileFormat;

    if(!context)
        return AudioFileFormat::Unknown;

    if(!context->iformat)
        return AudioFileFormat::Unknown;

    if(!audio)
        return AudioFileFormat::Unknown;

    const std::string container = toString(context->iformat->name);

    if(container == "wav")
        return AudioFileFormat::Wav;

    if(container == "flac")
        return AudioFileFormat::Flac;

    if(container == "mp3")
        return AudioFileFormat::Mp3;

    // AHX goes through the ADX demuxer too; Read tells them apart by the encoding type
    if(container == "adx")
        return AudioFileFormat::Adx;

    if(container == "ogg")
    {
        switch(audio->codec_id)
        {
            case AV_CODEC_ID_VORBIS: return AudioFileFormat::OggVorbis;
            case AV_CODEC_ID_OPUS:   return AudioFileFormat::Opus;
            case AV_CODEC_ID_FLAC:   return AudioFileFormat::Flac;
            default:                 return AudioFileFormat::Unknown;
        }
    }

    // One demuxer covers every MP4 flavour - it's only an M4A if nothing in it actually moves
    if(container.starts_with("mov,mp4") && !hasVideoStream(context))
        return AudioFileFormat::M4a;

    return AudioFileFormat::Unknown;
}

RetroFuturaGUI::WavMetadata RetroFuturaGUI::AudioMetadataReader::readWavDetails(std::string_view path, const AVCodecParameters* audio)
{
    RetroFuturaGUI::WavMetadata details;

    if(!audio)
        return details;

    details._BlockAlign = static_cast<u32>(std::max(audio->block_align, 0));

    std::ifstream file = openFile(path);

    if(!file)
        return details;

    const u64 fileSize = getFileSize(file);
    const std::vector<u8> riff = readBytes(file, 0, 12);

    // Plain little-endian RIFF only - RF64, BW64 and big-endian RIFX keep whatever FFmpeg found
    if(12 > riff.size() || 0 != std::memcmp(riff.data(), "RIFF", 4) || 0 != std::memcmp(riff.data() + 8, "WAVE", 4))
        return details;

    u64 offset { 12 };

    while(offset + 8 <= fileSize)
    {
        const std::vector<u8> chunk = readBytes(file, offset, 8);

        if(8 > chunk.size())
            break;

        const u64 chunkSize = AV_RL32(chunk.data() + 4);

        if(0 == std::memcmp(chunk.data(), "smpl", 4))
        {
            const std::vector<u8> sampler = readBytes(file, offset + 8, std::min<u64>(chunkSize, fileSize - offset - 8));

            // manufacturer, product, sample period, unity note, pitch fraction, SMPTE format and
            // offset, loop count, sampler data - then 24 bytes per loop
            if(0x24 > sampler.size())
                break;

            details._HasSamplerChunk = true;
            details._MidiUnityNote = AV_RL32(sampler.data() + 0x0C);

            const u32 loopCount = AV_RL32(sampler.data() + 0x1C);

            for(u32 i = 0; i < loopCount; ++i)
            {
                const uSize loopOffset = 0x24 + static_cast<uSize>(i) * 24;

                // the count can claim more loops than the chunk holds
                if(loopOffset + 24 > sampler.size())
                    break;

                const u8* loop = sampler.data() + loopOffset;

                details._Loops.push_back({
                    ._CuePointId = AV_RL32(loop),
                    ._Type = AV_RL32(loop + 4),
                    ._StartSample = AV_RL32(loop + 8),
                    ._EndSample = AV_RL32(loop + 12),
                    ._PlayCount = AV_RL32(loop + 20) });
            }

            break;
        }

        offset += 8 + chunkSize + (chunkSize & 1); // chunks are padded to an even size
    }

    return details;
}

RetroFuturaGUI::FlacMetadata RetroFuturaGUI::AudioMetadataReader::readFlacDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::FlacMetadata details;

    if(!audio)
        return details;

    const u8* info = audio->extradata;

    if(!info)
        return details;

    if(34 > audio->extradata_size)
        return details;

    details._MinBlockSize = AV_RB16(info);
    details._MaxBlockSize = AV_RB16(info + 2);
    details._MinFrameSize = AV_RB24(info + 4);
    details._MaxFrameSize = AV_RB24(info + 7);
    details._AudioMd5 = toHex(info + 18, 16);

    return details;
}

RetroFuturaGUI::Mp3Metadata RetroFuturaGUI::AudioMetadataReader::readMp3Details(std::string_view path)
{
    static constexpr std::array<std::string_view, 4> channelModes { "Stereo", "Joint Stereo", "Dual Channel", "Mono" };

    RetroFuturaGUI::Mp3Metadata details;
    std::ifstream file = openFile(path);

    if(!file)
        return details;

    const u64 fileSize = getFileSize(file);

    // ID3v1 is the last 128 bytes, starting with "TAG"
    if(128 <= fileSize)
    {
        const std::vector<u8> tail = readBytes(file, fileSize - 128, 3);
        details._HasId3v1 = 3 == tail.size() && 0 == std::memcmp(tail.data(), "TAG", 3);
    }

    // ID3v2 comes first: "ID3", major version, revision, flags, then a 28-bit size spread over four
    // bytes of seven bits each, not counting the 10-byte header or an optional 10-byte footer
    u64 audioStart { 0 };
    const std::vector<u8> id3 = readBytes(file, 0, 10);

    if(10 == id3.size() && 0 == std::memcmp(id3.data(), "ID3", 3))
    {
        details._Id3v2Version = id3[3];

        const u64 tagSize = (static_cast<u64>(id3[6] & 0x7F) << 21) | (static_cast<u64>(id3[7] & 0x7F) << 14)
            | (static_cast<u64>(id3[8] & 0x7F) << 7) | static_cast<u64>(id3[9] & 0x7F);
        const bool hasFooter = 0 != (id3[5] & 0x10);

        audioStart = 10 + tagSize + (hasFooter ? 10 : 0);
    }

    const std::vector<u8> window = readBytes(file, audioStart, 65536);

    for(uSize i = 0; i + 4 <= window.size(); ++i)
    {
        const u32 header = AV_RB32(window.data() + i);

        // 11 sync bits, then version, layer, bit rate and sample rate indices - reserved values mean
        // this isn't a frame header after all
        if((header & 0xFFE00000) != 0xFFE00000)
            continue;

        const u32 version = (header >> 19) & 3;         // 0 = 2.5, 1 reserved, 2 = MPEG 2, 3 = MPEG 1
        const u32 layer = (header >> 17) & 3;           // 1 = Layer III
        const u32 bitRateIndex = (header >> 12) & 0xF;
        const u32 sampleRateIndex = (header >> 10) & 3;

        if(1 == version || 1 != layer || 0 == bitRateIndex || 0xF == bitRateIndex || 3 == sampleRateIndex)
            continue;

        const u32 channelMode = (header >> 6) & 3;
        const bool mono = 3 == channelMode;
        const bool hasCrc = 0 == ((header >> 16) & 1);

        details._MpegVersion = 3 == version ? "1" : 2 == version ? "2" : "2.5";
        details._ChannelMode = channelModes[channelMode];

        // The Xing/Info header replaces the audio of the first frame, right after its side information
        const uSize sideInfoSize = 3 == version ? (mono ? 17 : 32) : (mono ? 9 : 17);
        const uSize xingOffset = i + 4 + (hasCrc ? 2 : 0) + sideInfoSize;
        const uSize vbriOffset = i + 4 + 32;

        if(xingOffset + 8 <= window.size()
            && (0 == std::memcmp(window.data() + xingOffset, "Xing", 4) || 0 == std::memcmp(window.data() + xingOffset, "Info", 4)))
        {
            details._VbrHeader.assign(reinterpret_cast<const char*>(window.data() + xingOffset), 4);
            details._IsVbr = details._VbrHeader == "Xing"; // LAME writes "Info" for constant bit rate

            // The LAME tag follows whichever optional Xing fields are present: frames, bytes, TOC, quality
            const u32 flags = AV_RB32(window.data() + xingOffset + 4);
            const uSize lameOffset = xingOffset + 8 + ((flags & 1) ? 4 : 0) + ((flags & 2) ? 4 : 0) + ((flags & 4) ? 100 : 0) + ((flags & 8) ? 4 : 0);

            if(lameOffset + 24 > window.size())
                break;

            const u8* lame = window.data() + lameOffset;

            if(0 != std::memcmp(lame, "LAME", 4) && 0 != std::memcmp(lame, "Lavf", 4) && 0 != std::memcmp(lame, "Lavc", 4))
                break;

            details._Encoder.assign(reinterpret_cast<const char*>(lame), 9);
            details._Encoder.erase(std::ranges::find(details._Encoder, '\0'), details._Encoder.end());

            while(!details._Encoder.empty() && details._Encoder.back() == ' ')
                details._Encoder.pop_back();

            // two 12-bit values: samples of silence added at the start, and at the end
            const u32 delays = AV_RB24(lame + 21);
            details._EncoderDelay = delays >> 12;
            details._Padding = delays & 0xFFF;
        }
        else if(vbriOffset + 4 <= window.size() && 0 == std::memcmp(window.data() + vbriOffset, "VBRI", 4))
        {
            details._VbrHeader = "VBRI";
            details._IsVbr = true;
        }

        break;
    }

    return details;
}

RetroFuturaGUI::OggVorbisMetadata RetroFuturaGUI::AudioMetadataReader::readVorbisDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::OggVorbisMetadata details;

    if(!audio)
        return details;

    const u8* data = audio->extradata;

    if(!data)
        return details;

    const uSize size = static_cast<uSize>(std::max(audio->extradata_size, 0));

    if(1 > size || 2 != data[0])
        return details;

    uSize offset { 1 };
    std::array<uSize, 2> headerSizes {};

    for(uSize& headerSize : headerSizes)
    {
        while(offset < size && 255 == data[offset])
        {
            headerSize += 255;
            ++offset;
        }

        if(offset >= size)
            return details;

        headerSize += data[offset++];
    }

    // Identification header: type 1, "vorbis", version, channels, sample rate, three bit rates, block sizes
    const u8* identification = data + offset;

    if(offset + 30 > size || 1 != identification[0] || 0 != std::memcmp(identification + 1, "vorbis", 6))
        return details;

    details._MaximumBitRate = static_cast<i32>(AV_RL32(identification + 16));
    details._NominalBitRate = static_cast<i32>(AV_RL32(identification + 20));
    details._MinimumBitRate = static_cast<i32>(AV_RL32(identification + 24));
    details._ShortBlockSize = 1u << (identification[28] & 0x0F);
    details._LongBlockSize = 1u << (identification[28] >> 4);

    // Comment header: type 3, "vorbis", then the vendor string FFmpeg reads past without keeping
    const uSize commentOffset = offset + headerSizes[0];

    if(commentOffset + 11 > size)
        return details;

    const u8* comment = data + commentOffset;

    if(3 != comment[0] || 0 != std::memcmp(comment + 1, "vorbis", 6))
        return details;

    const uSize vendorLength = AV_RL32(comment + 7);

    if(commentOffset + 11 + vendorLength > size)
        return details;

    details._Vendor.assign(reinterpret_cast<const char*>(comment + 11), vendorLength);

    return details;
}

RetroFuturaGUI::OpusMetadata RetroFuturaGUI::AudioMetadataReader::readOpusDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::OpusMetadata details;

    if(!audio)
        return details;

    const u8* head = audio->extradata;

    if(!head)
        return details;

    if(19 > audio->extradata_size || 0 != std::memcmp(head, "OpusHead", 8))
        return details;

    details._Version = head[8];
    details._PreSkip = AV_RL16(head + 10);
    details._InputSampleRate = AV_RL32(head + 12);
    details._OutputGainDb = static_cast<f32>(static_cast<i16>(AV_RL16(head + 16))) / 256.0f; // Q7.8 decibels
    details._ChannelMappingFamily = head[18];

    return details;
}

RetroFuturaGUI::M4aMetadata RetroFuturaGUI::AudioMetadataReader::readM4aDetails(const AVDictionary* fileTags, const AVCodecParameters* audio)
{
    RetroFuturaGUI::M4aMetadata details;

    if(!audio)
        return details;

    details._MajorBrand = findTag(fileTags, "major_brand");
    details._Profile = toString(avcodec_profile_name(audio->codec_id, audio->profile));

    return details;
}

RetroFuturaGUI::AdxMetadata RetroFuturaGUI::AudioMetadataReader::readAdxDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::AdxMetadata details;

    if(!audio)
        return details;

    const u8* header = audio->extradata;

    if(!header)
        return details;

    const uSize size = static_cast<uSize>(std::max(audio->extradata_size, 0));

    if(0x14 > size || 0x8000 != AV_RB16(header))
        return details;

    details._DataOffset = AV_RB16(header + 0x02) + 4u;
    details._EncodingType = header[0x04];
    details._BlockSize = header[0x05];
    details._BitsPerSample = header[0x06];
    details._Channels = header[0x07];
    details._SampleRate = AV_RB32(header + 0x08);
    details._TotalSamples = AV_RB32(header + 0x0C);
    details._HighpassFrequency = AV_RB16(header + 0x10);
    details._Version = header[0x12];

    const u32 flags = header[0x13];
    const u32 channels = header[0x07];

    details._EncryptionType = (0x08 == flags || 0x09 == flags) ? flags : 0;
    details._Decodable = 3 == details._EncodingType && 18 == details._BlockSize && 4 == details._BitsPerSample && 0 == details._EncryptionType;

    uSize loopsOffset { 0 };

    if(3 == details._Version)
    {
        loopsOffset = 0x14;

        // the header only carries loop information when it's long enough to
        if(details._DataOffset < 6 + loopsOffset + 0x18)
            return details;
    }
    else if(4 == details._Version)
    {
        // Version 4 puts per-channel decoder history first, and optionally an AINF block
        const uSize historySize = 1 < channels ? 4 * channels : 8;
        const uSize ainfOffset = 0x18 + historySize + 4;
        uSize ainfSize { 0 };

        if(ainfOffset + 8 <= size && 0 == std::memcmp(header + ainfOffset, "AINF", 4))
            ainfSize = AV_RB32(header + ainfOffset + 4);

        loopsOffset = 0x18 + historySize;

        if(details._DataOffset < 6 + ainfSize + loopsOffset + 0x18)
            return details;
    }
    else
    {
        return details; // version 5 has no loop block
    }

    if(loopsOffset + 0x18 > size)
        return details;

    // 16-bit alignment padding and a 16-bit flag first, then the fields below
    details._HasLoopInfo = true;
    details._LoopEnabled = 0 != AV_RB32(header + loopsOffset + 0x04);
    details._LoopStartSample = AV_RB32(header + loopsOffset + 0x08);
    details._LoopStartByte = AV_RB32(header + loopsOffset + 0x0C);
    details._LoopEndSample = AV_RB32(header + loopsOffset + 0x10);
    details._LoopEndByte = AV_RB32(header + loopsOffset + 0x14);

    return details;
}

const AVStream* RetroFuturaGUI::AudioMetadataReader::findAudioStream(const RetroFuturaGUI::MediaSource& source)
{
    const AVStream* best = source.GetStream(source.FindBestStream(AVMEDIA_TYPE_AUDIO));

    if(best)
        return best;

    // av_find_best_stream skips an audio stream without a channel count, and that is exactly how one
    // looks after FFmpeg's decoder refused it - type 4 ADX and AHX, for instance. Its metadata is
    // still worth reading, so fall back to the first audio stream there is.
    for(u32 i = 0; i < source.GetStreamCount(); ++i)
    {
        const AVStream* stream = source.GetStream(static_cast<i32>(i));

        if(!stream)
            continue;

        if(!stream->codecpar)
            continue;

        if(stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            return stream;
    }

    return nullptr;
}

std::optional<RetroFuturaGUI::AudioMetadata> RetroFuturaGUI::AudioMetadataReader::Read(std::string_view path)
{
    MediaSource source;

    if(!source.Open(path))
        return std::nullopt;

    const AVFormatContext* context = source.GetFormatContext();

    if(!context)
        return std::nullopt;

    if(!context->iformat)
        return std::nullopt;

    const AVStream* stream = findAudioStream(source);

    if(!stream)
        return std::nullopt;

    const AVCodecParameters* audio = stream->codecpar;

    if(!audio)
        return std::nullopt;

    AudioMetadata metadata;

    metadata._Container = toString(context->iformat->name);
    metadata._Codec = toString(avcodec_get_name(audio->codec_id));
    metadata._CodecIsFree = IsCodecFree(audio->codec_id);

    metadata._SampleRate = static_cast<u32>(std::max(audio->sample_rate, 0));
    metadata._Channels = static_cast<u32>(std::max(audio->ch_layout.nb_channels, 0));
    metadata._ChannelLayout = describeLayout(audio->ch_layout);
    metadata._SampleFormat = toString(av_get_sample_fmt_name(static_cast<AVSampleFormat>(audio->format)));
    metadata._BitsPerSample = static_cast<u32>(std::max(0 < audio->bits_per_raw_sample ? audio->bits_per_raw_sample : av_get_bits_per_sample(audio->codec_id), 0));
    metadata._BitRate = 0 < audio->bit_rate ? audio->bit_rate : context->bit_rate;

    // The stream's own duration where the container records one, the file's otherwise
    if(0 < stream->duration)
    {
        metadata._Seconds = static_cast<f64>(stream->duration) * av_q2d(stream->time_base);

        if(0 < audio->sample_rate)
            metadata._TotalSamples = static_cast<u64>(av_rescale_q(stream->duration, stream->time_base, AVRational { .num = 1, .den = audio->sample_rate }));
    }
    else if(0 < context->duration)
    {
        metadata._Seconds = static_cast<f64>(context->duration) / AV_TIME_BASE;
        metadata._TotalSamples = static_cast<u64>(std::llround(metadata._Seconds * metadata._SampleRate));
    }

    metadata._DurationIsEstimate = context->duration_estimation_method == AVFMT_DURATION_FROM_BITRATE;

    const AVDictionary* fileTags = context->metadata;
    const AVDictionary* streamTags = stream->metadata;

    // FFmpeg maps each format's own tag names onto these - ID3's TIT2, MP4's ©nam and a Vorbis
    // comment's TITLE all arrive as "title"
    metadata._Title = findTag(fileTags, streamTags, { "title" });
    metadata._Artist = findTag(fileTags, streamTags, { "artist" });
    metadata._Album = findTag(fileTags, streamTags, { "album" });
    metadata._AlbumArtist = findTag(fileTags, streamTags, { "album_artist", "albumartist" });
    metadata._Date = findTag(fileTags, streamTags, { "date", "year" });
    metadata._Genre = findTag(fileTags, streamTags, { "genre" });
    metadata._Track = findTag(fileTags, streamTags, { "track", "tracknumber" });
    metadata._Disc = findTag(fileTags, streamTags, { "disc", "discnumber" });
    metadata._Composer = findTag(fileTags, streamTags, { "composer" });
    metadata._Comment = findTag(fileTags, streamTags, { "comment", "description" });

    appendTags(fileTags, metadata._Tags);
    appendTags(streamTags, metadata._Tags);

    metadata._Chapters = readChapters(context);
    readCoverArt(context, metadata);

    metadata._Format = classify(context, audio);

    switch(metadata._Format)
    {
        case AudioFileFormat::Wav:
        {
            WavMetadata details = readWavDetails(path, audio);

            if(!details._Loops.empty())
            {
                // smpl stores the last sample that still plays; AudioLoop wants the one after it
                const WavSampleLoop& loop = details._Loops.front();
                metadata._Loop = AudioLoop { ._StartSample = loop._StartSample, ._EndSample = static_cast<u64>(loop._EndSample) + 1, ._Source = "WAV smpl chunk" };
            }

            metadata._Details = std::move(details);
            break;
        }

        case AudioFileFormat::Flac:
            metadata._Details = readFlacDetails(audio);
            break;

        case AudioFileFormat::Mp3:
        {
            Mp3Metadata details = readMp3Details(path);

            // With a LAME tag, FFmpeg's decoder skips the encoder delay plus the MP3 decoder's own
            // 529-sample delay at the start, but trims the end only down to padding - 529. So the
            // start always loses delay + 529 and the end at most what the padding leaves over,
            // which makes the decoded length frames * samples-per-frame - delay - max(padding, 529).
            if(!details._Encoder.empty() && !metadata._DurationIsEstimate)
            {
                const u64 trimmed = static_cast<u64>(details._EncoderDelay) + std::max<u64>(details._Padding, 529);

                metadata._TotalSamples = metadata._TotalSamples > trimmed ? metadata._TotalSamples - trimmed : 0;
                metadata._Seconds = toSeconds(metadata._TotalSamples, metadata._SampleRate);
            }

            metadata._Details = std::move(details);
            break;
        }

        case AudioFileFormat::OggVorbis:
            metadata._Details = readVorbisDetails(audio);
            break;

        case AudioFileFormat::Opus:
        {
            const OpusMetadata details = readOpusDetails(audio);

            // Ogg Opus positions count the pre-skip samples too, which the decoder throws away
            if(metadata._TotalSamples > details._PreSkip)
            {
                metadata._TotalSamples -= details._PreSkip;
                metadata._Seconds = toSeconds(metadata._TotalSamples, metadata._SampleRate);
            }

            metadata._Details = details;
            break;
        }

        case AudioFileFormat::M4a:
        {
            M4aMetadata details = readM4aDetails(fileTags, audio);

            // iTunes keeps its gapless information in "iTunSMPB": hex fields, the second and third
            // being priming and padding, the fourth the length that's left - the exact one
            const std::vector<u64> gapless = parseHexFields(findTag(fileTags, streamTags, { "iTunSMPB" }));

            if(4 <= gapless.size())
            {
                details._EncoderDelay = static_cast<u32>(gapless[1]);
                details._Padding = static_cast<u32>(gapless[2]);

                if(0 < gapless[3])
                {
                    metadata._TotalSamples = gapless[3];
                    metadata._Seconds = toSeconds(metadata._TotalSamples, metadata._SampleRate);
                }
            }

            metadata._Details = std::move(details);
            break;
        }

        case AudioFileFormat::Adx:
        {
            AdxMetadata details = readAdxDetails(audio);

            if(0 == metadata._Channels)
            {
                metadata._Channels = details._Channels;

                // FFmpeg's layout went along with its channel count, so describe the usual one instead
                AVChannelLayout layout {};
                av_channel_layout_default(&layout, static_cast<i32>(details._Channels));
                metadata._ChannelLayout = describeLayout(layout);
                av_channel_layout_uninit(&layout);
            }

            if(0 == metadata._SampleRate)
                metadata._SampleRate = details._SampleRate;

            // AHX shares the ADX header, and the encoding type is the only thing that tells them
            // apart. FFmpeg files it as ADX audio, which it isn't, so its codec, bit depth and bit
            // rate figures don't apply.
            if(0x10 == details._EncodingType || 0x11 == details._EncodingType)
            {
                metadata._Format = AudioFileFormat::Ahx;
                metadata._Codec = "ahx";
                metadata._BitsPerSample = 0;
                metadata._BitRate = 0;
            }

            // The header's sample count is exact; the demuxer sets no duration, leaving FFmpeg to
            // guess one from the bit rate
            if(0 < details._TotalSamples && 0 < metadata._SampleRate)
            {
                metadata._TotalSamples = details._TotalSamples;
                metadata._Seconds = toSeconds(details._TotalSamples, metadata._SampleRate);
                metadata._DurationIsEstimate = false;
            }

            if(details._LoopEnabled)
                metadata._Loop = AudioLoop { ._StartSample = details._LoopStartSample, ._EndSample = details._LoopEndSample, ._Source = "ADX header" };

            metadata._Details = details;
            break;
        }

        default:
            break;
    }

    // Formats without a loop block of their own can still carry the tag convention
    if(!metadata._Loop)
        metadata._Loop = readLoopTags(fileTags, streamTags);

    return metadata;
}