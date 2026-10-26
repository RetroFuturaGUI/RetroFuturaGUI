#pragma once

extern "C"
{
    #include <libavutil/frame.h>
}

namespace RetroFuturaGUI
{
    /// @brief Owns an AVFrame for its whole lifetime
    class MediaFrame
    {
    public:
        MediaFrame();
        MediaFrame(const MediaFrame&) = delete;
        MediaFrame(MediaFrame&& other) noexcept;
        ~MediaFrame();
        MediaFrame& operator=(const MediaFrame&) = delete;
        MediaFrame& operator=(MediaFrame&& other) noexcept;

        /// @brief Call ofter constructor
        bool IsValid() const;
        AVFrame* operator->() const;
        AVFrame* Get() const;
        void UnrefFrame();

    private:
        AVFrame* _frame { nullptr };
    };
}