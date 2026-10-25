#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    /// @brief Clips drawing to a box for as long as it lives, then gives the previous clip back. The box is narrowed to a clip
    class ScissorClip
    {
    public:
        /// @brief Clips to box: left, bottom, width and height in framebuffer pixels (see IWidget::framebufferBox).
        explicit ScissorClip(const glm::ivec4& box);
        ~ScissorClip();
        ScissorClip(const ScissorClip&) = delete;
        ScissorClip(ScissorClip&&) = delete;
        auto operator =(const ScissorClip&) = delete;
        auto operator =(ScissorClip&&) = delete;

    private:
        bool _wasEnabled { false };
        i32 _previousBox[4] { 0, 0, 0, 0 };
    };
}