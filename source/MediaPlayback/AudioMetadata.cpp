#include "AudioMetadata.hpp"
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

// FFmpeg returns null for names it doesn't know
static std::string toString(const char* text)
{
    if(!text)
        return {};

    return text;
}

static std::string toHex(const u8* bytes, const size_t count)
{
    if(!bytes)
        return {};

    std::string hex;

    for(size_t i = 0; i < count; ++i)
        hex += std::format("{:02x}", bytes[i]);

    return hex;
}

static std::optional<u64> parseUnsigned(const std::string& text)
{
    u64 value { 0 };
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);

    if(error != std::errc())
        return std::nullopt;

    return value;
}

// Splits a run of space-separated hexadecimal numbers, stopping at the first that doesn't parse
static std::vector<u64> parseHexFields(const std::string& text)
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

static f64 toSeconds(const u64 samples, const u32 sampleRate)
{
    if(sampleRate == 0)
        return 0.0;

    return static_cast<f64>(samples) / sampleRate;
}

static std::string describeLayout(const AVChannelLayout& layout)
{
    std::array<char, 128> buffer {};

    if(av_channel_layout_describe(&layout, buffer.data(), buffer.size()) <= 0)
        return {};

    return buffer.data();
}

// Opened through std::filesystem::path so a UTF-8 path reaches the wide Win32 file API intact,
// whatever the process code page is
static std::ifstream openFile(std::string_view path)
{
    const std::u8string utf8(path.begin(), path.end());

    return std::ifstream(std::filesystem::path(utf8), std::ios::binary);
}

static u64 getFileSize(std::ifstream& file)
{
    file.clear();
    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();

    return size > 0 ? static_cast<u64>(size) : 0;
}

// Returns fewer bytes than asked for when the file ends first
static std::vector<u8> readBytes(std::ifstream& file, const u64 offset, const u64 count)
{
    std::vector<u8> bytes(static_cast<size_t>(count));

    file.clear();
    file.seekg(static_cast<std::streamoff>(offset));
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(count));
    bytes.resize(static_cast<size_t>(std::max<std::streamsize>(file.gcount(), 0)));

    return bytes;
}

static std::string findTag(const AVDictionary* dictionary, const char* key)
{
    // av_dict_get matches keys case-insensitively, so "title" also finds a Vorbis comment's "TITLE"
    const AVDictionaryEntry* entry = av_dict_get(dictionary, key, nullptr, 0);

    if(!entry)
        return {};

    return toString(entry->value);
}

// Tries each key in order, in the file's tags first and then the audio stream's - Ogg keeps its
// comments on the stream, while most other formats keep them on the file
static std::string findTag(const AVDictionary* fileTags, const AVDictionary* streamTags, const std::initializer_list<const char*> keys)
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

static void appendTags(const AVDictionary* dictionary, std::vector<std::pair<std::string, std::string>>& tags)
{
    for(const AVDictionaryEntry* entry = av_dict_iterate(dictionary, nullptr); entry; entry = av_dict_iterate(dictionary, entry))
    {
        const bool alreadyListed = std::ranges::any_of(tags, [entry](const std::pair<std::string, std::string>& tag)
        {
            return av_strcasecmp(tag.first.c_str(), entry->key) == 0;
        });

        if(alreadyListed)
            continue;

        tags.emplace_back(toString(entry->key), toString(entry->value));
    }
}

// The loop-point convention of RPG Maker and many other engines, stored as ordinary tags. It turns
// up in Ogg Vorbis mostly, but works the same way in Opus and FLAC.
static std::optional<RetroFuturaGUI::AudioLoop> readLoopTags(const AVDictionary* fileTags, const AVDictionary* streamTags)
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

static std::vector<RetroFuturaGUI::AudioChapter> readChapters(const AVFormatContext* context)
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

// FFmpeg exposes embedded cover art as an extra video stream holding a single picture
static void readCoverArt(const AVFormatContext* context, RetroFuturaGUI::AudioMetadata& metadata)
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

        metadata._CoverArt.assign(stream->attached_pic.data, stream->attached_pic.data + stream->attached_pic.size);
        return; // the first picture only - it's the front cover in practice
    }
}

static bool hasVideoStream(const AVFormatContext* context)
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

static RetroFuturaGUI::AudioFileFormat classify(const AVFormatContext* context, const AVCodecParameters* audio)
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

    // AHX goes through the ADX demuxer too; ReadAudioMetadata tells them apart by the encoding type
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

// Walks the RIFF chunk list for "smpl", which FFmpeg skips. It often sits after the audio data, so
// the walk seeks from header to header instead of reading the file.
static RetroFuturaGUI::WavMetadata readWavDetails(std::string_view path, const AVCodecParameters* audio)
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
    if(riff.size() < 12 || std::memcmp(riff.data(), "RIFF", 4) != 0 || std::memcmp(riff.data() + 8, "WAVE", 4) != 0)
        return details;

    u64 offset { 12 };

    while(offset + 8 <= fileSize)
    {
        const std::vector<u8> chunk = readBytes(file, offset, 8);

        if(chunk.size() < 8)
            break;

        const u64 chunkSize = AV_RL32(chunk.data() + 4);

        if(std::memcmp(chunk.data(), "smpl", 4) == 0)
        {
            const std::vector<u8> sampler = readBytes(file, offset + 8, std::min<u64>(chunkSize, fileSize - offset - 8));

            // manufacturer, product, sample period, unity note, pitch fraction, SMPTE format and
            // offset, loop count, sampler data - then 24 bytes per loop
            if(sampler.size() < 0x24)
                break;

            details._HasSamplerChunk = true;
            details._MidiUnityNote = AV_RL32(sampler.data() + 0x0C);

            const u32 loopCount = AV_RL32(sampler.data() + 0x1C);

            for(u32 i = 0; i < loopCount; ++i)
            {
                const size_t loopOffset = 0x24 + static_cast<size_t>(i) * 24;

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

// The FLAC demuxer hands over the 34-byte STREAMINFO block as extradata
static RetroFuturaGUI::FlacMetadata readFlacDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::FlacMetadata details;

    if(!audio)
        return details;

    const u8* info = audio->extradata;

    if(!info)
        return details;

    if(audio->extradata_size < 34)
        return details;

    details._MinBlockSize = AV_RB16(info);
    details._MaxBlockSize = AV_RB16(info + 2);
    details._MinFrameSize = AV_RB24(info + 4);
    details._MaxFrameSize = AV_RB24(info + 7);
    details._AudioMd5 = toHex(info + 18, 16);

    return details;
}

// FFmpeg reads the Xing and LAME headers for its own use but keeps what it learns private, so the
// first frame is parsed again here
static RetroFuturaGUI::Mp3Metadata readMp3Details(std::string_view path)
{
    static constexpr std::array<std::string_view, 4> channelModes { "Stereo", "Joint Stereo", "Dual Channel", "Mono" };

    RetroFuturaGUI::Mp3Metadata details;
    std::ifstream file = openFile(path);

    if(!file)
        return details;

    const u64 fileSize = getFileSize(file);

    // ID3v1 is the last 128 bytes, starting with "TAG"
    if(fileSize >= 128)
    {
        const std::vector<u8> tail = readBytes(file, fileSize - 128, 3);
        details._HasId3v1 = tail.size() == 3 && std::memcmp(tail.data(), "TAG", 3) == 0;
    }

    // ID3v2 comes first: "ID3", major version, revision, flags, then a 28-bit size spread over four
    // bytes of seven bits each, not counting the 10-byte header or an optional 10-byte footer
    u64 audioStart { 0 };
    const std::vector<u8> id3 = readBytes(file, 0, 10);

    if(id3.size() == 10 && std::memcmp(id3.data(), "ID3", 3) == 0)
    {
        details._Id3v2Version = id3[3];

        const u64 tagSize = (static_cast<u64>(id3[6] & 0x7F) << 21) | (static_cast<u64>(id3[7] & 0x7F) << 14)
            | (static_cast<u64>(id3[8] & 0x7F) << 7) | static_cast<u64>(id3[9] & 0x7F);
        const bool hasFooter = (id3[5] & 0x10) != 0;

        audioStart = 10 + tagSize + (hasFooter ? 10 : 0);
    }

    const std::vector<u8> window = readBytes(file, audioStart, 65536);

    for(size_t i = 0; i + 4 <= window.size(); ++i)
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

        if(version == 1 || layer != 1 || bitRateIndex == 0 || bitRateIndex == 0xF || sampleRateIndex == 3)
            continue;

        const u32 channelMode = (header >> 6) & 3;
        const bool mono = channelMode == 3;
        const bool hasCrc = ((header >> 16) & 1) == 0;

        details._MpegVersion = version == 3 ? "1" : version == 2 ? "2" : "2.5";
        details._ChannelMode = channelModes[channelMode];

        // The Xing/Info header replaces the audio of the first frame, right after its side information
        const size_t sideInfoSize = version == 3 ? (mono ? 17 : 32) : (mono ? 9 : 17);
        const size_t xingOffset = i + 4 + (hasCrc ? 2 : 0) + sideInfoSize;
        const size_t vbriOffset = i + 4 + 32;

        if(xingOffset + 8 <= window.size()
            && (std::memcmp(window.data() + xingOffset, "Xing", 4) == 0 || std::memcmp(window.data() + xingOffset, "Info", 4) == 0))
        {
            details._VbrHeader.assign(reinterpret_cast<const char*>(window.data() + xingOffset), 4);
            details._IsVbr = details._VbrHeader == "Xing"; // LAME writes "Info" for constant bit rate

            // The LAME tag follows whichever optional Xing fields are present: frames, bytes, TOC, quality
            const u32 flags = AV_RB32(window.data() + xingOffset + 4);
            const size_t lameOffset = xingOffset + 8 + ((flags & 1) ? 4 : 0) + ((flags & 2) ? 4 : 0) + ((flags & 4) ? 100 : 0) + ((flags & 8) ? 4 : 0);

            if(lameOffset + 24 > window.size())
                break;

            const u8* lame = window.data() + lameOffset;

            if(std::memcmp(lame, "LAME", 4) != 0 && std::memcmp(lame, "Lavf", 4) != 0 && std::memcmp(lame, "Lavc", 4) != 0)
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
        else if(vbriOffset + 4 <= window.size() && std::memcmp(window.data() + vbriOffset, "VBRI", 4) == 0)
        {
            details._VbrHeader = "VBRI";
            details._IsVbr = true;
        }

        break;
    }

    return details;
}

// The Ogg demuxer packs all three Vorbis headers into extradata, Xiph-laced: a count byte (2), the
// sizes of the first two headers as runs of bytes ending below 255, then the headers back to back
static RetroFuturaGUI::OggVorbisMetadata readVorbisDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::OggVorbisMetadata details;

    if(!audio)
        return details;

    const u8* data = audio->extradata;

    if(!data)
        return details;

    const size_t size = static_cast<size_t>(std::max(audio->extradata_size, 0));

    if(size < 1 || data[0] != 2)
        return details;

    size_t offset { 1 };
    std::array<size_t, 2> headerSizes {};

    for(size_t& headerSize : headerSizes)
    {
        while(offset < size && data[offset] == 255)
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

    if(offset + 30 > size || identification[0] != 1 || std::memcmp(identification + 1, "vorbis", 6) != 0)
        return details;

    details._MaximumBitRate = static_cast<i32>(AV_RL32(identification + 16));
    details._NominalBitRate = static_cast<i32>(AV_RL32(identification + 20));
    details._MinimumBitRate = static_cast<i32>(AV_RL32(identification + 24));
    details._ShortBlockSize = 1u << (identification[28] & 0x0F);
    details._LongBlockSize = 1u << (identification[28] >> 4);

    // Comment header: type 3, "vorbis", then the vendor string FFmpeg reads past without keeping
    const size_t commentOffset = offset + headerSizes[0];

    if(commentOffset + 11 > size)
        return details;

    const u8* comment = data + commentOffset;

    if(comment[0] != 3 || std::memcmp(comment + 1, "vorbis", 6) != 0)
        return details;

    const size_t vendorLength = AV_RL32(comment + 7);

    if(commentOffset + 11 + vendorLength > size)
        return details;

    details._Vendor.assign(reinterpret_cast<const char*>(comment + 11), vendorLength);

    return details;
}

// Extradata is the OpusHead packet: magic, version, channels, pre-skip, input rate, gain, mapping family
static RetroFuturaGUI::OpusMetadata readOpusDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::OpusMetadata details;

    if(!audio)
        return details;

    const u8* head = audio->extradata;

    if(!head)
        return details;

    if(audio->extradata_size < 19 || std::memcmp(head, "OpusHead", 8) != 0)
        return details;

    details._Version = head[8];
    details._PreSkip = AV_RL16(head + 10);
    details._InputSampleRate = AV_RL32(head + 12);
    details._OutputGainDb = static_cast<f32>(static_cast<i16>(AV_RL16(head + 16))) / 256.0f; // Q7.8 decibels
    details._ChannelMappingFamily = head[18];

    return details;
}

static RetroFuturaGUI::M4aMetadata readM4aDetails(const AVDictionary* fileTags, const AVCodecParameters* audio)
{
    RetroFuturaGUI::M4aMetadata details;

    if(!audio)
        return details;

    details._MajorBrand = findTag(fileTags, "major_brand");
    details._Profile = toString(avcodec_profile_name(audio->codec_id, audio->profile));

    return details;
}

// The ADX demuxer hands over the whole header, up to and including the "(c)CRI" signature. FFmpeg
// reads channels and sample rate from it and nothing else - loop points included. The loop block
// offsets follow vgmstream, the reference for CRI formats.
static RetroFuturaGUI::AdxMetadata readAdxDetails(const AVCodecParameters* audio)
{
    RetroFuturaGUI::AdxMetadata details;

    if(!audio)
        return details;

    const u8* header = audio->extradata;

    if(!header)
        return details;

    const size_t size = static_cast<size_t>(std::max(audio->extradata_size, 0));

    if(size < 0x14 || AV_RB16(header) != 0x8000)
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

    details._EncryptionType = (flags == 0x08 || flags == 0x09) ? flags : 0;
    details._Decodable = details._EncodingType == 3 && details._BlockSize == 18 && details._BitsPerSample == 4 && details._EncryptionType == 0;

    size_t loopsOffset { 0 };

    if(details._Version == 3)
    {
        loopsOffset = 0x14;

        // the header only carries loop information when it's long enough to
        if(details._DataOffset < 6 + loopsOffset + 0x18)
            return details;
    }
    else if(details._Version == 4)
    {
        // Version 4 puts per-channel decoder history first, and optionally an AINF block
        const size_t historySize = channels > 1 ? 4 * channels : 8;
        const size_t ainfOffset = 0x18 + historySize + 4;
        size_t ainfSize { 0 };

        if(ainfOffset + 8 <= size && std::memcmp(header + ainfOffset, "AINF", 4) == 0)
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
    details._LoopEnabled = AV_RB32(header + loopsOffset + 0x04) != 0;
    details._LoopStartSample = AV_RB32(header + loopsOffset + 0x08);
    details._LoopStartByte = AV_RB32(header + loopsOffset + 0x0C);
    details._LoopEndSample = AV_RB32(header + loopsOffset + 0x10);
    details._LoopEndByte = AV_RB32(header + loopsOffset + 0x14);

    return details;
}

static const AVStream* findAudioStream(const RetroFuturaGUI::MediaSource& source)
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

std::optional<RetroFuturaGUI::AudioMetadata> RetroFuturaGUI::ReadAudioMetadata(std::string_view path)
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
    metadata._BitsPerSample = static_cast<u32>(std::max(audio->bits_per_raw_sample > 0 ? audio->bits_per_raw_sample : av_get_bits_per_sample(audio->codec_id), 0));
    metadata._BitRate = audio->bit_rate > 0 ? audio->bit_rate : context->bit_rate;

    // The stream's own duration where the container records one, the file's otherwise
    if(stream->duration > 0)
    {
        metadata._Seconds = static_cast<f64>(stream->duration) * av_q2d(stream->time_base);

        if(audio->sample_rate > 0)
            metadata._TotalSamples = static_cast<u64>(av_rescale_q(stream->duration, stream->time_base, AVRational { .num = 1, .den = audio->sample_rate }));
    }
    else if(context->duration > 0)
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

            if(gapless.size() >= 4)
            {
                details._EncoderDelay = static_cast<u32>(gapless[1]);
                details._Padding = static_cast<u32>(gapless[2]);

                if(gapless[3] > 0)
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

            if(metadata._Channels == 0)
            {
                metadata._Channels = details._Channels;

                // FFmpeg's layout went along with its channel count, so describe the usual one instead
                AVChannelLayout layout {};
                av_channel_layout_default(&layout, static_cast<i32>(details._Channels));
                metadata._ChannelLayout = describeLayout(layout);
                av_channel_layout_uninit(&layout);
            }

            if(metadata._SampleRate == 0)
                metadata._SampleRate = details._SampleRate;

            // AHX shares the ADX header, and the encoding type is the only thing that tells them
            // apart. FFmpeg files it as ADX audio, which it isn't, so its codec, bit depth and bit
            // rate figures don't apply.
            if(details._EncodingType == 0x10 || details._EncodingType == 0x11)
            {
                metadata._Format = AudioFileFormat::Ahx;
                metadata._Codec = "ahx";
                metadata._BitsPerSample = 0;
                metadata._BitRate = 0;
            }

            // The header's sample count is exact; the demuxer sets no duration, leaving FFmpeg to
            // guess one from the bit rate
            if(details._TotalSamples > 0 && metadata._SampleRate > 0)
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