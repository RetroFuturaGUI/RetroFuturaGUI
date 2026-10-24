#include "VideoStill.hpp"
#include "Decoder.hpp"
#include "Frame.hpp"
#include "MediaSource.hpp"
#include "Packet.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>

extern "C"
{
    #include <libavutil/pixdesc.h>
    #include <libswscale/swscale.h>
}

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::DecodeVideoFrame(std::string_view path, const i64 milliseconds, const i32 maxSide, const bool flipVertically)
{
    MediaSource source;

    if(!source.Open(path))
        return std::nullopt;

    const AVStream* stream { source.GetStream(source.FindVideoStream()) };

    if(!stream)
        return std::nullopt;

    Decoder decoder;

    if(!decoder.Open(stream)) //refuses codecs that aren't free
        return std::nullopt;

    Packet packet;
    Frame
        frame,
        latest;

    if(!packet.IsValid() || !frame.IsValid() || !latest.IsValid())
        return std::nullopt;

    //Past the end, decoding starts from the end and runs to the last picture. A container's length can end where the last
    //picture starts, so stopping at that moment would show the one before.
    const AVFormatContext* context { source.GetFormatContext() };
    const i64 durationMilliseconds { context && 0 < context->duration ? context->duration / (AV_TIME_BASE / 1000) : 0 };
    const bool isPastEnd { 0 < durationMilliseconds && durationMilliseconds <= milliseconds };
    const i64
        moment { isPastEnd ? durationMilliseconds : (std::max)(milliseconds, i64 { 0 }) },
        target { isPastEnd ? (std::numeric_limits<i64>::max)() : av_rescale_q(moment, AVRational { .num = 1, .den = 1000 }, stream->time_base) + (AV_NOPTS_VALUE != stream->start_time ? stream->start_time : 0) };

    //One try: seeks, then decodes forward to the first picture at or past the moment, or the last one if the video ends sooner.
    //It fails when it finds no picture, or when its first picture is already past the moment.
    const auto decodeFrom { [&](const i64 startMilliseconds) -> bool
        {
            if(0 > source.Seek(startMilliseconds))
                return false;

            decoder.Flush();
            av_frame_unref(latest.Get());

            constexpr uSize decodedFrameLimit { 300 }; //a keyframe far before the moment shows a picture a little early rather than taking long
            uSize decodedFramesCount { 0 };
            bool
                hasPicture { false },
                isAtMoment { false },
                isDrained { false };

            while(!isAtMoment && !isDrained && decodedFrameLimit > decodedFramesCount)
            {
                if(0 > source.ReadPacket(packet.Get()))
                {
                    decoder.SendPacket(nullptr); //the end of the file: the decoder lets go of the pictures it holds back
                    isDrained = true;
                }
                else
                {
                    const bool isVideo { packet->stream_index == decoder.GetStreamIndex() };

                    if(isVideo)
                        decoder.SendPacket(packet.Get());

                    av_packet_unref(packet.Get());

                    if(!isVideo)
                        continue;
                }

                //Pictures come out in the order they're shown
                while(!isAtMoment && 0 == decoder.ReceiveFrame(frame.Get()))
                {
                    ++decodedFramesCount;

                    //A seek into the middle of a group of pictures gives pictures missing what they're built on, until a keyframe
                    if(!hasPicture && 0 == (frame->flags & AV_FRAME_FLAG_KEY))
                    {
                        av_frame_unref(frame.Get());
                        continue;
                    }

                    const i64 time { frame->best_effort_timestamp };

                    if(!hasPicture && 0 < startMilliseconds && AV_NOPTS_VALUE != time && target < time)
                        return false; //landed past the moment; from the start, the first picture is the earliest there is

                    av_frame_unref(latest.Get());
                    av_frame_move_ref(latest.Get(), frame.Get());
                    hasPicture = true;
                    isAtMoment = AV_NOPTS_VALUE == time || target <= time;
                }
            }

            return hasPicture;
        } };

    //Seeking lands on a keyframe before the moment where the file has an index of them. Without one, an MPEG program stream
    //say, it can land past that keyframe, so each try starts further back, up to the start.
    bool hasPicture { false };

    for(i64 back { 0 }; !hasPicture; back = 0 == back ? 1000 : 2 * back)
    {
        const i64 start { (std::max)(i64 { 0 }, moment - back) };
        hasPicture = decodeFrom(start);

        if(0 == start)
            break;
    }

    if(!hasPicture)
        return std::nullopt;

    const AVFrame* picture { latest.Get() };

    if(0 >= picture->width || 0 >= picture->height)
        return std::nullopt;

    //Shown at its display shape, since some videos store wide pixels; the stream's word goes before the frame's
    const AVRational
        streamPixelAspect { stream->sample_aspect_ratio },
        framePixelAspect { picture->sample_aspect_ratio };
    const f64
        pixelAspect { 0 < streamPixelAspect.num && 0 < streamPixelAspect.den ? av_q2d(streamPixelAspect) : 0 < framePixelAspect.num && 0 < framePixelAspect.den ? av_q2d(framePixelAspect) : 1.0 },
        displayWidth { static_cast<f64>(picture->width) * pixelAspect },
        displayHeight { static_cast<f64>(picture->height) },
        longest { (std::max)(displayWidth, displayHeight) },
        scale { 0 < maxSide && static_cast<f64>(maxSide) < longest ? static_cast<f64>(maxSide) / longest : 1.0 };
    const i32
        width { (std::max)(1, static_cast<i32>(std::lround(displayWidth * scale))) },
        height { (std::max)(1, static_cast<i32>(std::lround(displayHeight * scale))) };
    const AVPixelFormat pixelFormat { static_cast<AVPixelFormat>(picture->format) };
    const AVPixFmtDescriptor* format { av_pix_fmt_desc_get(pixelFormat) };

    if(!format)
        return std::nullopt;

    //SWS_AREA averages the pixels each target pixel covers (prevents small thumbnail from shimmering (hopefully))
    std::unique_ptr<SwsContext, decltype(&sws_freeContext)> converter { sws_getContext(picture->width, picture->height, pixelFormat, width, height, AV_PIX_FMT_RGBA, SWS_AREA, nullptr, nullptr, nullptr), &sws_freeContext };

    if(!converter)
        return std::nullopt;

    //The video's own color matrix and range, so colors come out as a usable format
    if(0 == (format->flags & AV_PIX_FMT_FLAG_RGB))
    {
        const bool isFullRange { AVCOL_RANGE_JPEG == picture->color_range };
        sws_setColorspaceDetails(converter.get(), sws_getCoefficients(picture->colorspace), isFullRange, sws_getCoefficients(SWS_CS_DEFAULT), 1, 0, 1 << 16, 1 << 16);
    }

    Frame converted;

    if(!converted.IsValid())
        return std::nullopt;

    converted->format = AV_PIX_FMT_RGBA;
    converted->width = width;
    converted->height = height;

    if(0 > av_frame_get_buffer(converted.Get(), 0))
        return std::nullopt;

    if(0 >= sws_scale(converter.get(), picture->data, picture->linesize, 0, picture->height, converted->data, converted->linesize))
        return std::nullopt;

    //Copied row by row, flipped if asked: FFmpeg may leave padding after each converted row, and the image's rows have none
    const uSize rowBytes { static_cast<uSize>(width) * 4 };
    DecodedImage image
    {
        ._Pixels = std::vector<u8>(rowBytes * static_cast<uSize>(height)),
        ._Resolution = glm::i32vec2(width, height),
        ._SourceResolution = glm::i32vec2(picture->width, picture->height), //as stored, before wide pixels are widened
        ._IsFlipped = flipVertically
    };

    for(i32 row = 0; row < height; ++row)
    {
        const i32 sourceRow { flipVertically ? height - 1 - row : row };
        std::memcpy(image._Pixels.data() + static_cast<uSize>(row) * rowBytes, converted->data[0] + static_cast<ptrdiff_t>(sourceRow) * converted->linesize[0], rowBytes);
    }

    return image;
}