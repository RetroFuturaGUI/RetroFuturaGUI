#include "VideoMetadata.hpp"
#include "MediaSource.hpp"

std::optional<RetroFuturaGUI::VideoMetadata> RetroFuturaGUI::ReadVideoMetadata(std::string_view path)
{
    MediaSource source;

    if(!source.Open(path))
        return std::nullopt;

    const AVFormatContext* context { source.GetFormatContext() };

    if(!context)
        return std::nullopt;

    //The first moving picture; songs carry their cover art as a one-picture video stream, which doesn't count
    const AVStream* video { nullptr };

    for(u32 index = 0; index < source.GetStreamCount() && !video; ++index)
    {
        const AVStream* stream { source.GetStream(static_cast<i32>(index)) };
        const bool isPicture { stream && stream->codecpar && AVMEDIA_TYPE_VIDEO == stream->codecpar->codec_type };

        if(isPicture && 0 == (stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
            video = stream;
    }

    if(!video)
        return std::nullopt;

    const AVCodecParameters* picture { video->codecpar };
    VideoMetadata metadata {};
    metadata._Container = context->iformat && context->iformat->name ? context->iformat->name : "";
    metadata._VideoCodec = avcodec_get_name(picture->codec_id);
    metadata._VideoCodecIsFree = IsCodecFree(picture->codec_id);
    metadata._Width = static_cast<u32>(std::max(picture->width, 0));
    metadata._Height = static_cast<u32>(std::max(picture->height, 0));
    metadata._BitRate = context->bit_rate;

    const AVRational rate { 0 != video->avg_frame_rate.den ? video->avg_frame_rate : video->r_frame_rate };
    metadata._FramesPerSecond = 0 != rate.den ? av_q2d(rate) : 0.0;

    //The stream's own duration where the container records one, the file's otherwise
    if(0 < video->duration)
        metadata._Seconds = static_cast<f64>(video->duration) * av_q2d(video->time_base);
    else if(0 < context->duration)
        metadata._Seconds = static_cast<f64>(context->duration) / AV_TIME_BASE;

    const AVDictionaryEntry* title { av_dict_get(context->metadata, "title", nullptr, 0) };
    metadata._Title = title && title->value ? title->value : "";

    const AVStream* audio { source.GetStream(source.FindBestStream(AVMEDIA_TYPE_AUDIO)) };

    if(!audio)
        return metadata; //a silent video

    const AVCodecParameters* sound { audio->codecpar };

    if(!sound)
        return metadata;

    metadata._HasAudio = true;
    metadata._AudioCodec = avcodec_get_name(sound->codec_id);
    metadata._AudioCodecIsFree = IsCodecFree(sound->codec_id);
    metadata._SampleRate = static_cast<u32>(std::max(sound->sample_rate, 0));
    metadata._Channels = static_cast<u32>(std::max(sound->ch_layout.nb_channels, 0));
    return metadata;
}