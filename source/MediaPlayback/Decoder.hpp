#pragma once
#include "config.hpp"
#include "DecodingPolicy.hpp"

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    //#include <libswresample/swresample.h>
    #include <libavutil/opt.h>
}

namespace RetroFuturaGUI
{
    /// @brief Decodes one stream of a MediaSource: compressed packets in, frames out. Audio and
    /// video decode through the same calls - only the contents of the frames differ. Opening is
    /// refused for any stream whose codec is not on the FreeCodecs list.
    class Decoder
    {
    public:
        Decoder() = default;
        Decoder(const Decoder&) = delete;
        Decoder& operator=(const Decoder&) = delete;
        Decoder(Decoder&& other) noexcept;
        Decoder& operator=(Decoder&& other) noexcept;
        ~Decoder();

        /// @brief Opens a decoder for the stream. The stream is only read during this call, so the
        /// decoder keeps no pointer into the MediaSource afterwards.
        /// Returns false for a null stream, a codec that isn't free, or one FFmpeg can't decode.
        bool Open(const AVStream* stream);

        /// @brief Releases the decoder. Idempotent - safe on a closed decoder.
        void Close();

        /// @brief Feeds one packet of this decoder's stream. nullptr marks the end of the stream,
        /// which makes the decoder release the frames it is still holding.
        /// @return 0 on success. AVERROR(EAGAIN) if frames have to be received first - the packet
        /// was not taken and must be sent again. Any other negative value is an error.
        i32 SendPacket(const AVPacket* packet);

        /// @brief Takes the next decoded frame. The caller owns the frame and can reuse it, since
        /// FFmpeg unrefs it before filling it again.
        /// @return 0 if a frame was produced, AVERROR(EAGAIN) if the decoder needs another packet,
        /// AVERROR_EOF once it has been fully drained. Any other negative value is an error.
        i32 ReceiveFrame(AVFrame* frame);

        /// @brief Throws away every buffered frame. Call it after each seek, and before decoding
        /// again once the stream has been drained to AVERROR_EOF.
        void Flush();

        /// @brief Returns whether a decoder is open.
        bool IsOpen() const { return _codecContext != nullptr; }

        /// @brief Returns the index of the decoded stream, to compare against AVPacket::stream_index.
        /// -1 while closed.
        i32 GetStreamIndex() const { return _streamIndex; }

        /// @brief Returns the stream's time base: frame->pts * av_q2d(GetTimeBase()) is the frame's
        /// time in seconds.
        AVRational GetTimeBase() const { return _timeBase; }

    private:
        AVCodecContext* _codecContext { nullptr };
        AVRational _timeBase { 0, 1 };
        i32 _streamIndex { -1 };
    };
}
