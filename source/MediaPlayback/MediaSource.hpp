#pragma once
#include "config.hpp"
#include "DecodingPolicy.hpp"
#include <string_view>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    //#include <libswresample/swresample.h>
    #include <libavutil/opt.h>
}

namespace RetroFuturaGUI
{
    /// @brief Owns the demuxer for one media file
    class MediaSource
    {
    public:
        MediaSource() = default;
        MediaSource(const MediaSource&) = delete;
        MediaSource& operator=(const MediaSource&) = delete;
        MediaSource(MediaSource&& other) noexcept;
        MediaSource& operator=(MediaSource&& other) noexcept;
        ~MediaSource();

        /// @brief Opens a media file and reads its stream information. Choosing a stream and
        /// decoding it are left to the caller - see FindBestStream, GetStream and Decoder.
        /// Returns false if the file can't be opened or its streams can't be read.
        bool Open(std::string_view path);

        /// @brief Closes the file. Idempotent - safe on a closed source
        void Close();

        /// @brief Returns whether a file is open
        bool IsOpen() const;

        /// @brief Returns the index of the best stream of the given type, or -1 if there is none.
        i32 FindBestStream(const AVMediaType type) const;

        /// @brief Returns the stream at index, or nullptr if the index is out of range or nothing is open.
        const AVStream* GetStream(const i32 index) const;

        /// @brief Returns how many streams the file has, 0 while nothing is open.
        u32 GetStreamCount() const;

        /// @brief Read-only access to the demuxer, for inspecting what FFmpeg found - file-level tags,
        /// chapters, the container name. Reading and seeking go through ReadPacket and Seek instead.
        const AVFormatContext* GetFormatContext() const { return _formatContext; }

        /// @brief Reads the next packet in the file, from whichever stream comes next - audio and
        /// video are interleaved, so route it by packet->stream_index. The caller owns the packet
        /// and has to av_packet_unref it once it's been used.
        /// @return 0 on success, AVERROR_EOF at the end of the file, any other negative value on error.
        i32 ReadPacket(AVPacket* packet);

        /// @brief Seeks to a position in milliseconds. Lands on the keyframe at or before it, not
        /// exactly on it - for an exact position, decode forward and drop frames that are still early.
        /// Every Decoder reading this source has to be flushed afterwards.
        /// @return 0 on success, a negative AVERROR code on failure.
        i32 Seek(const i64 milliseconds);

    private:
        AVFormatContext* _formatContext { nullptr };
    };
}
