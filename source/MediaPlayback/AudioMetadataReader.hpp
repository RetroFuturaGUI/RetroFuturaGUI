#pragma once
#include "AudioMetadata.hpp"
#include <cstddef>
#include <initializer_list>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

//FFmpeg's types, only declared, so including this header doesn't bring in FFmpeg's headers
struct AVChannelLayout;
struct AVCodecParameters;
struct AVDictionary;
struct AVFormatContext;
struct AVStream;

namespace RetroFuturaGUI
{
    class MediaSource;

    /// @brief Reads what an audio file says about itself into an AudioMetadata, without playing it.
    class AudioMetadataReader
    {
    public:
        /// @brief Reads what a file says about itself, without playing it. Returns nullopt if the file cannot be opened or it has
        ///        no audio stream. Files whose codec isn't free read as well - check _CodecIsFree before handing one to Decoder.
        static std::optional<AudioMetadata> Read(std::string_view path);

    private:
        AudioMetadataReader() = delete;
        AudioMetadataReader(const AudioMetadataReader&) = delete;
        AudioMetadataReader(AudioMetadataReader&&) = delete;
        auto operator =(const AudioMetadataReader&) = delete;
        auto operator =(AudioMetadataReader&&) = delete;

        /// @brief Returns the text as a string, or an empty one for null, which FFmpeg returns for names it doesn't know.
        static std::string toString(const char* text);

        /// @brief Returns the bytes as lowercase hexadecimal digits, two per byte.
        static std::string toHex(const u8* bytes, const size_t count);

        /// @brief Returns the decimal number the text starts with, or nullopt if it starts with none.
        static std::optional<u64> parseUnsigned(const std::string& text);

        /// @brief Splits a run of space-separated hexadecimal numbers, stopping at the first that doesn't parse.
        static std::vector<u64> parseHexFields(const std::string& text);

        /// @brief Returns how long the samples last at the sample rate, or 0 for a sample rate of 0.
        static f64 toSeconds(const u64 samples, const u32 sampleRate);

        /// @brief Returns FFmpeg's name for the channel layout, such as "stereo", or an empty string if it gives none.
        static std::string describeLayout(const AVChannelLayout& layout);

        /// @brief Opens the file for reading bytes. It goes through std::filesystem::path, so a UTF-8 path reaches the wide Win32
        ///        file API intact, whatever the process code page is.
        static std::ifstream openFile(std::string_view path);

        /// @brief Returns the file's size in bytes, or 0 when it can't be told. Leaves the read position at the end.
        static u64 getFileSize(std::ifstream& file);

        /// @brief Reads count bytes from offset. Returns fewer when the file ends first.
        static std::vector<u8> readBytes(std::ifstream& file, const u64 offset, const u64 count);

        /// @brief Returns the tag's value, or an empty string when the dictionary doesn't hold it.
        static std::string findTag(const AVDictionary* dictionary, const char* key);

        /// @brief Tries each key in order, in the file's tags first and then the audio stream's. Ogg keeps its comments on the
        ///        stream, while most other formats keep them on the file.
        static std::string findTag(const AVDictionary* fileTags, const AVDictionary* streamTags, const std::initializer_list<const char*> keys);

        /// @brief Adds the dictionary's tags to tags, leaving out keys already listed there, whatever their case.
        static void appendTags(const AVDictionary* dictionary, std::vector<std::pair<std::string, std::string>>& tags);

        /// @brief Reads loop points from LOOPSTART with LOOPLENGTH or LOOPEND tags, the convention of RPG Maker and many other
        ///        engines. It turns up in Ogg Vorbis mostly, but works the same way in Opus and FLAC.
        static std::optional<AudioLoop> readLoopTags(const AVDictionary* fileTags, const AVDictionary* streamTags);

        /// @brief Returns the file's chapters with their start, end (-1 when unknown) and title.
        static std::vector<AudioChapter> readChapters(const AVFormatContext* context);

        /// @brief Returns where an embedded picture's own JPEG or PNG signature starts within its first bytes, or 0 for another
        ///        format. Some taggers leave a stray byte before the picture - an extra null after an ID3 picture's description, say -
        ///        and FFmpeg keeps it. Readers that follow the standard, Windows Explorer among them, then find no picture.
        static size_t findPictureStart(const u8* data, const size_t size);

        /// @brief Fills the cover art fields from the first embedded picture. FFmpeg exposes embedded cover art as an extra video
        ///        stream holding a single picture.
        static void readCoverArt(const AVFormatContext* context, AudioMetadata& metadata);

        /// @brief Returns whether the file has a video stream that moves. Cover art is a video stream as well, but a still one.
        static bool hasVideoStream(const AVFormatContext* context);

        /// @brief Decides the format from the container and, for Ogg, from the codec inside it.
        static AudioFileFormat classify(const AVFormatContext* context, const AVCodecParameters* audio);

        /// @brief Walks the RIFF chunk list for "smpl", which FFmpeg skips. It often sits after the audio data, so the walk seeks
        ///        from header to header instead of reading the file.
        static WavMetadata readWavDetails(std::string_view path, const AVCodecParameters* audio);

        /// @brief Reads the 34-byte STREAMINFO block, which the FLAC demuxer hands over as extradata.
        static FlacMetadata readFlacDetails(const AVCodecParameters* audio);

        /// @brief Parses the first frame's Xing and LAME headers again. FFmpeg reads them for its own use but keeps what it learns
        ///        private.
        static Mp3Metadata readMp3Details(std::string_view path);

        /// @brief Reads the Vorbis headers. The Ogg demuxer packs all three into extradata, Xiph-laced: a count byte (2), the sizes
        ///        of the first two headers as runs of bytes ending below 255, then the headers back to back.
        static OggVorbisMetadata readVorbisDetails(const AVCodecParameters* audio);

        /// @brief Reads the OpusHead packet from extradata: magic, version, channels, pre-skip, input rate, gain, mapping family.
        static OpusMetadata readOpusDetails(const AVCodecParameters* audio);

        /// @brief Reads the MP4 major brand and the codec's profile.
        static M4aMetadata readM4aDetails(const AVDictionary* fileTags, const AVCodecParameters* audio);

        /// @brief Reads the ADX header, which the demuxer hands over whole, up to and including the "(c)CRI" signature. FFmpeg reads
        ///        channels and sample rate from it and nothing else - loop points included. The loop block offsets follow vgmstream,
        ///        the reference for CRI formats.
        static AdxMetadata readAdxDetails(const AVCodecParameters* audio);

        /// @brief Returns FFmpeg's best audio stream, or else the first audio stream there is.
        static const AVStream* findAudioStream(const MediaSource& source);
    };
}