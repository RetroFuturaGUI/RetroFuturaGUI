#pragma once
#include "config.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

/*ToDo:
CoverArtCodec enum class
Decoded album covers*/

namespace RetroFuturaGUI
{
    /// @brief The audio formats AudioMetadata tells apart. Decided from the container and the codec
    /// inside it, never from the file extension
    enum class AudioFileFormat : u32
    {
        Unknown,    // opened fine but none of the below - only the common metadata is filled
        Wav,
        Flac,       // native FLAC, or FLAC inside Ogg
        Mp3,
        OggVorbis,  // Vorbis, Opus or FLAC.
        Opus,
        M4a,        // AAC or ALAC in an MP4 container
        Adx,
        Ahx         // CRI's MPEG audio behind an ADX-style header - its metadata reads, its audio doesn't decode yet
        //Todo:
        //Hca,
        //WebM,
        //Aiff
    };

    /// @brief Returns a display name for the format.
    constexpr std::string_view GetAudioFileFormatName(const AudioFileFormat format)
    {
        switch(format)
        {
            case AudioFileFormat::Wav:       return "WAV";
            case AudioFileFormat::Flac:      return "FLAC";
            case AudioFileFormat::Mp3:       return "MP3";
            case AudioFileFormat::OggVorbis: return "Ogg Vorbis";
            case AudioFileFormat::Opus:      return "Opus";
            case AudioFileFormat::M4a:       return "M4A";
            case AudioFileFormat::Adx:       return "ADX";
            case AudioFileFormat::Ahx:       return "AHX";
            default:                         return "Unknown";
        }
    }

    /// @brief A loop region in samples. The end is exclusive - the first sample after the loop, where
    /// playback jumps back to the start - whichever convention the file itself uses.
    struct AudioLoop
    {
        u64
            _StartSample { 0 },
            _EndSample { 0 };
        std::string _Source;        // where it was found, e.g. "ADX header" or "WAV smpl chunk"
    };

    /// @brief A chapter, a FLAC cue sheet track, or a WAV cue marker.
    struct AudioChapter
    {
        f64
            _StartSeconds { 0.0 },
            _EndSeconds { -1.0 };   // FFmpeg ends a marker that has none at the next one or the end of the file; -1 if it can't
        std::string _Title;
    };

    /// @brief One loop of a WAV smpl chunk, as stored.
    struct WavSampleLoop
    {
        u32
            _CuePointId { 0 },
            _Type { 0 },            // 0 forward, 1 alternating, 2 backward
            _StartSample { 0 },
            _EndSample { 0 },       // inclusive - the smpl chunk stores the last sample that still plays
            _PlayCount { 0 };       // 0 loops forever
    };

    struct WavMetadata
    {
        u32 _BlockAlign { 0 };
        bool _HasSamplerChunk { false };
        u32 _MidiUnityNote { 0 };   // the sampler's root key - only meaningful with a smpl chunk
        std::vector<WavSampleLoop> _Loops;
    };

    /// @brief From the STREAMINFO block.
    struct FlacMetadata
    {
        u32
            _MinBlockSize { 0 },
            _MaxBlockSize { 0 },
            _MinFrameSize { 0 },    // in bytes; 0 when the encoder didn't record it
            _MaxFrameSize { 0 };
        std::string _AudioMd5;      // hex MD5 of the decoded audio; all zeros when the encoder didn't compute one
    };

    struct Mp3Metadata
    {
        u32 _Id3v2Version { 0 };    // major version 2, 3 or 4; 0 without an ID3v2 tag
        bool _HasId3v1 { false };
        std::string
            _MpegVersion,           // "1", "2" or "2.5"
            _ChannelMode,           // "Stereo", "Joint Stereo", "Dual Channel" or "Mono"
            _VbrHeader;             // "Xing" (VBR), "Info" (CBR), "VBRI" (VBR), empty when there is none
        bool _IsVbr { false };
        std::string _Encoder;       // from the LAME tag, e.g. "LAME3.100"
        u32
            _EncoderDelay { 0 },    // gapless playback: silent samples the encoder put at the start...
            _Padding { 0 };         // ...and at the end
    };

    /// @brief From the Vorbis identification and comment headers.
    struct OggVorbisMetadata
    {
        std::string _Vendor;        // the encoder library, e.g. "Xiph.Org libVorbis I 20200704 (Reducing Environment)"
        i32
            _NominalBitRate { 0 },  // encoder hints rather than measurements; 0 or -1 when unset
            _MinimumBitRate { 0 },
            _MaximumBitRate { 0 };
        u32
            _ShortBlockSize { 0 },
            _LongBlockSize { 0 };
    };

    /// @brief From the OpusHead header.
    struct OpusMetadata
    {
        u32
            _Version { 0 },
            _PreSkip { 0 },         // samples at 48 kHz to discard at the start
            _InputSampleRate { 0 }; // the rate before encoding - informational only, Opus always decodes at 48 kHz
        f32 _OutputGainDb { 0.0f };
        u32 _ChannelMappingFamily { 0 };
    };

    struct M4aMetadata
    {
        std::string
            _MajorBrand,            // usually "M4A "
            _Profile;               // the AAC profile, e.g. "LC" or "HE-AAC"; empty for ALAC
        u32
            _EncoderDelay { 0 },    // gapless playback, from iTunes' iTunSMPB tag: priming samples at the start...
            _Padding { 0 };         // ...and padding at the end
    };

    /// @brief From the ADX header - AHX uses the same layout, so it fills this as well.
    struct AdxMetadata
    {
        u32
            _Version { 0 },         // 3 and 4 can carry loop information, 5 doesn't
            _EncodingType { 0 },    // 3 standard, 4 exponential scale, 2 fixed coefficients, 0x10/0x11 AHX
            _BlockSize { 0 },
            _BitsPerSample { 0 },
            _HighpassFrequency { 0 },
            _EncryptionType { 0 },  // 0, or 8/9 - encrypted audio needs the game's key and decodes to noise without it
            _DataOffset { 0 },
            _Channels { 0 },        // as the header states them - FFmpeg loses the channel count when its decoder
            _SampleRate { 0 },      // refuses the stream, so these are the reliable figures
            _TotalSamples { 0 };
        bool
            _HasLoopInfo { false },
            _LoopEnabled { false };
        u32
            _LoopStartSample { 0 },
            _LoopStartByte { 0 },
            _LoopEndSample { 0 },
            _LoopEndByte { 0 };
        bool _Decodable { false };  // whether FFmpeg's decoder takes it: type 3, 18-byte blocks, 4 bits, not encrypted
    };

    /// @brief Everything a file says about itself. What every format has lives here; what only one
    /// format carries lives in _Details, as that format's own struct.
    struct AudioMetadata
    {
        AudioFileFormat _Format { AudioFileFormat::Unknown };
        std::string
            _Container, // FFmpeg's demuxer name, e.g. "ogg" or "mov,mp4,m4a,3gp,3g2,mj2"
            _Codec;
        bool _CodecIsFree { false }; // whether the codec is royalityfree

        f64 _Seconds { 0.0 };
        u64 _TotalSamples { 0 }; // what Decoder actually produces, with encoder delay and padding already removed
        bool _DurationIsEstimate { false }; // guessed from the bit rate, typical for an MP3 without a Xing header

        u32
            _SampleRate { 0 },
            _Channels { 0 },
            _BitsPerSample { 0 }; // 0 for lossy codecs and such where it has no meaning
        std::string
            _ChannelLayout, // e.g. "stereo" or "5.1"
            _SampleFormat;  // what the decoder outputs, e.g. "s16" or "fltp"
        i64 _BitRate { 0 };

        std::string
            _Title,
            _Artist,
            _Album,
            _AlbumArtist,
            _Date,
            _Genre,
            _Track,
            _Disc,
            _Composer,
            _Comment;
        std::vector<std::pair<std::string, std::string>> _Tags; // every tag in the file, the ones above included
        std::vector<AudioChapter> _Chapters;
        std::string _CoverArtCodec; // e.g. "mjpeg" or "png"; empty without cover art
        u32
            _CoverArtWidth { 0 },
            _CoverArtHeight { 0 };
        std::vector<u8> _CoverArt; // the image as embedded, still encoded
        std::optional<AudioLoop> _Loop; // from wherever the format keeps its loop points
        std::variant<std::monostate, WavMetadata, FlacMetadata, Mp3Metadata, OggVorbisMetadata, OpusMetadata, M4aMetadata, AdxMetadata> _Details;
    };

    /// @brief Reads what a file says about itself, without playing it. Returns nullopt if 
    /// the file cannot be opened or it has no audio stream. Files whose codec isn't free read as well - check
    /// _CodecIsFree before handing one to Decoder.
    std::optional<AudioMetadata> ReadAudioMetadata(std::string_view path);
}