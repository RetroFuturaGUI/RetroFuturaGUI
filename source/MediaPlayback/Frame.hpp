#pragma once

extern "C"
{
    #include <libavutil/frame.h>
}

namespace RetroFuturaGUI
{
    /// @brief Owns an AVFrame for its whole lifetime
    class Frame
    {
    public:
        Frame();
        Frame(const Frame&) = delete;
        Frame(Frame&& other) noexcept;
        ~Frame();
        Frame& operator=(const Frame&) = delete;
        Frame& operator=(Frame&& other) noexcept;

        /// @brief Call ofter constructor
        bool IsValid() const;
        AVFrame* operator->() const;
        AVFrame* Get() const;
        void UnrefFrame();

    private:
        AVFrame* _frame { nullptr };
    };
}