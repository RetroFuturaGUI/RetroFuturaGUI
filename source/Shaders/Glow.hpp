#pragma once
#include "IncludeHelper.hpp"
#include "StateFillStyle.hpp"

namespace RetroFuturaGUI
{
    class Text;
    class GlowRectangle;

    /// @brief Light around a background, a border or a text: full at their edge and fading outward over its width. A border
    ///        glows from its outer edge, and from its inner edge too while the widget has a gap between border and background.
    ///        Its colors take every fill and gradient, per color state like the part it glows around, and follow that part's
    ///        state. Enabled is white at three quarters opacity by default. Reached by GetGlow():
    ///        button.GetBorder().GetGlow().Show(true). Hidden until Show(true)
    class Glow final : public StateFillStyle
    {
    public:
        Glow();
        ~Glow() = default;

        /// @brief Shows or hides the glow. Hidden by default
        void Show(const bool show);

        /// @brief Returns whether the glow is shown
        bool IsShown() const;

        /// @brief Sets how far the glow reaches out of the edge, in pixels, fading from full at the edge to nothing there. 8 by
        ///        default. A text renders its glow's glyphs once per width, so a text's width is best not changed every frame
        void SetWidth(const f32 width);
        f32 GetWidth() const;

    private:
        friend class Text; //draws it.
        friend class GlowRectangle; //draws it.

        f32 _width { 8.0f };
        bool _isShown { false };
    };
}
