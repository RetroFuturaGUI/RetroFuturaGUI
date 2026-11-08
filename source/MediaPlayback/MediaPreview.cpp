#include "MediaPreview.hpp"
#include "AudioMetadataReader.hpp"
#include "VideoMetadata.hpp"
#include "VideoStill.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <format>

RetroFuturaGUI::MediaPreviewResult RetroFuturaGUI::MediaPreview::ReadPreview(const MediaKind kind, std::string_view path, const i32 pictureSide)
{
    return readPreview(kind, path, pictureSide, VideoPicture::Decoded);
}

RetroFuturaGUI::MediaPreviewResult RetroFuturaGUI::MediaPreview::ReadPreviewDetails(const MediaKind kind, std::string_view path, const i32 pictureSide)
{
    return readPreview(kind, path, pictureSide, VideoPicture::LeftOut);
}

RetroFuturaGUI::MediaPreviewResult RetroFuturaGUI::MediaPreview::readPreview(const MediaKind kind, std::string_view path, const i32 pictureSide, const VideoPicture videoPicture)
{
    MediaPreviewResult result {};

    switch(kind)
    {
        case MediaKind::Image:
            result._Picture = Texture::DecodeFile(std::filesystem::path(std::u8string(path.begin(), path.end())), true, pictureSide);

            if(!result._Picture)
            {
                result._Message = "Cannot read this image";
                break;
            }

            addDetail(result, "Pixels", std::format("{} x {}", result._Picture->_SourceResolution.x, result._Picture->_SourceResolution.y));
        break;
        case MediaKind::Audio:
            if(!readAudio(path, pictureSide, result))
                result._Message = "Cannot read this file";
        break;
        case MediaKind::Video:
        {
            const std::optional<VideoMetadata> video { ReadVideoMetadata(path) };

            // A WebM or Matroska file may hold only sound (shown like a song)
            if(!video && readAudio(path, pictureSide, result))
                break;

            if(!video)
            {
                result._Message = "Cannot read this video";
                break;
            }

            addDetail(result, "Title", video->_Title);
            addDetail(result, "Length", 0.0 < video->_Seconds ? FormatDuration(video->_Seconds) : std::string {});
            addDetail(result, "Pixels", std::format("{} x {}", video->_Width, video->_Height));
            addDetail(result, "Frames", 0.0 < video->_FramesPerSecond ? std::format("{:g} fps", std::round(video->_FramesPerSecond * 1000.0) / 1000.0) : std::string {});
            addDetail(result, "Video", CodecName(video->_VideoCodec));
            addDetail(result, "Audio", video->_HasAudio ? std::format("{}, {:g} kHz, {} channels", CodecName(video->_AudioCodec), video->_SampleRate / 1000.0, video->_Channels) : std::string("None"));
            addDetail(result, "Format", video->_Container);

            //The same check decides the frame now and playing later
            if(!video->_VideoCodecIsAllowed)
            {
                result._Message = std::format("No preview:\n{} isn't royalty-free", CodecName(video->_VideoCodec));
                break;
            }

            if(VideoPicture::LeftOut == videoPicture)
                break; //the caller has the picture already

            //The pane shows this picture itself, so the program never decodes the file to show it; in the decoder helper a file that
            //crashes the decoder takes only the helper down
            result._Picture = DecodeVideoFrame(path, static_cast<i64>(video->_Seconds * 100.0), StillMoment::Keyframe, pictureSide, true); //10% in, in milliseconds
            result._CanShowFrame = result._Picture.has_value();

            if(!result._CanShowFrame)
                result._Message = "Can't show this video";
        }
        break;
    }

    return result;
}

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::MediaPreview::ReadThumbnail(const MediaKind kind, std::string_view path, const i32 pictureSide)
{
    switch(kind)
    {
        case MediaKind::Image:
            return Texture::DecodeFile(std::filesystem::path(std::u8string(path.begin(), path.end())), true, pictureSide);
        case MediaKind::Audio:
            return readCoverArt(path, pictureSide);
        case MediaKind::Video:
        {
            const std::optional<VideoMetadata> video { ReadVideoMetadata(path) };

            // A WebM or Matroska file may hold only sound and a song's cover art
            if(!video)
                return readCoverArt(path, pictureSide);

            if(!video->_VideoCodecIsAllowed)
                return std::nullopt;

            return DecodeVideoFrame(path, static_cast<i64>(video->_Seconds * 100.0), StillMoment::Keyframe, pictureSide, true); //10% in, in milliseconds
        }
    }

    return std::nullopt;
}

bool RetroFuturaGUI::MediaPreview::readAudio(std::string_view path, const i32 pictureSide, MediaPreviewResult& result)
{
    const std::optional<AudioMetadata> audio { AudioMetadataReader::Read(path) };

    if(!audio)
        return false;

    // A tracker module's format by its tracker's name, "Impulse Tracker 2.14" rather than "Tracker module"
    const ModuleMetadata* module { std::get_if<ModuleMetadata>(&audio->_Details) };
    std::string format { AudioFileFormat::Unknown == audio->_Format ? CodecName(audio->_Codec) : std::string(GetAudioFileFormatName(audio->_Format)) };

    if(module && !module->_TypeName.empty())
        format = module->_TypeName;

    addDetail(result, "Title", audio->_Title);
    addDetail(result, "Artist", audio->_Artist);
    addDetail(result, "Album", audio->_Album);
    addDetail(result, "Length", 0.0 < audio->_Seconds ? FormatDuration(audio->_Seconds) : std::string {});
    addDetail(result, "Year", audio->_Date);
    addDetail(result, "Genre", audio->_Genre);
    addDetail(result, "Track", audio->_Track);
    addDetail(result, "Format", std::move(format));
    addDetail(result, "Rate", 0 < audio->_SampleRate ? std::format("{:g} kHz", audio->_SampleRate / 1000.0) : std::string {});
    addDetail(result, "Channels", 0 < audio->_Channels ? std::to_string(audio->_Channels) : std::string {});
    addDetail(result, "Bit rate", 0 < audio->_BitRate ? std::format("{} kbps", (audio->_BitRate + 500) / 1000) : std::string {});

    if(!audio->_CoverArt.empty())
        result._Picture = DecodeCoverArt(audio->_CoverArt, audio->_CoverArtCodec, pictureSide, true);

    if(!result._Picture)
        result._Message = "No cover art";

    return true;
}

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::MediaPreview::readCoverArt(std::string_view path, const i32 pictureSide)
{
    const std::optional<AudioMetadata> audio { AudioMetadataReader::Read(path) };

    if(!audio || audio->_CoverArt.empty())
        return std::nullopt;

    return DecodeCoverArt(audio->_CoverArt, audio->_CoverArtCodec, pictureSide, true);
}

void RetroFuturaGUI::MediaPreview::addDetail(MediaPreviewResult& result, std::string_view key, std::string value)
{
    if(value.empty())
        return;

    result._Details.emplace_back(std::string(key), std::move(value));
}

std::string RetroFuturaGUI::MediaPreview::FormatDuration(const f64 seconds)
{
    const i64
        totalSeconds { static_cast<i64>(std::llround(seconds)) },
        hours { totalSeconds / 3600 },
        minutes { totalSeconds / 60 % 60 },
        rest { totalSeconds % 60 };

    if(0 < hours)
        return std::format("{}:{:02}:{:02}", hours, minutes, rest);

    return std::format("{}:{:02}", minutes, rest);
}

std::string RetroFuturaGUI::MediaPreview::CodecName(std::string_view ffmpegName)
{
    constexpr std::array<std::pair<std::string_view, std::string_view>, 20> names
    {{
        { "h264", "H.264" }, { "hevc", "HEVC" }, { "vvc", "VVC" }, { "vc1", "VC-1" }, { "mpeg4", "MPEG-4 Part 2" },
        { "mpeg2video", "MPEG-2" }, { "mpeg1video", "MPEG-1" }, { "vp8", "VP8" }, { "vp9", "VP9" }, { "av1", "AV1" },
        { "theora", "Theora" }, { "ffv1", "FFV1" }, { "mjpeg", "Motion JPEG" }, { "aac", "AAC" }, { "mp3", "MP3" },
        { "mp2", "MP2" }, { "opus", "Opus" }, { "vorbis", "Vorbis" }, { "flac", "FLAC" }, { "pcm_s16le", "PCM" }
    }};
    const auto known { std::find_if(names.begin(), names.end(), [ffmpegName](const auto& name) { return name.first == ffmpegName; }) };
    return names.end() != known ? std::string(known->second) : std::string(ffmpegName);
}