#pragma once
#include "IncludeHelper.hpp"
#include "FillStyle.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    class Text;
    class GlowRectangle;

    /// @brief Light around a background, a border or a text: full at their edge and fading outward over its width. A border
    ///        glows from its outer edge, and from its inner edge too while the widget has a gap between border and background.
    ///        Its colors take every fill and gradient, per color state like the part it glows around, and follow that part's
    ///        state. Reached by GetGlow(): button.GetBorder().GetGlow().Show(true). Hidden until Show(true)
    class Glow final : public FillStyle
    {
    public:
        Glow();
        Glow(const Glow&) = delete;
        Glow(Glow&&) = delete;
        ~Glow() = default;
        auto operator =(const Glow&) = delete;
        auto operator =(Glow&&) = delete;

        /// @brief Shows or hides the glow. Hidden by default
        void Show(const bool show);

        /// @brief Returns whether the glow is shown
        bool IsShown() const;

        /// @brief Sets how far the glow reaches out of the edge, in pixels, fading from full at the edge to nothing there. 8 by
        ///        default. A text renders its glow's glyphs once per width, so a text's width is best not changed every frame
        void SetWidth(const f32 width);
        f32 GetWidth() const;

        /// @brief Sets the glow's colors for a color state. A state without colors of its own shows the Enabled ones, so
        ///        setting Enabled alone gives one glow for every state. Enabled is white at three quarters opacity by default
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set for a color state, empty when it shows the Enabled ones
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        using FillStyle::SetFillType;
        using FillStyle::SetGradientOffset;
        using FillStyle::SetGradientAnimationSpeed;
        using FillStyle::SetGradientDegree;
        using FillStyle::SetGradientRotationSpeed;
        using FillStyle::GetGradientOffset;
        using FillStyle::GetGradientDegree;

    //Called by the part it glows around
        /// @brief Shows the colors of the given state, the Enabled ones when the state has none
        void SetColorState(const ColorState state);

    private:
        friend class Text; //draws it.
        friend class GlowRectangle; //draws it.

        /// @brief The colors set for a state.
        std::vector<glm::vec4>& colorsOf(const ColorState state);
        const std::vector<glm::vec4>& colorsOf(const ColorState state) const;

        /// @brief Hands the colors of the state shown to the fill.
        void applyColors();

        std::vector<glm::vec4>
            _colorsEnabled { glm::vec4(1.0f, 1.0f, 1.0f, 0.75f) }, //never empty: every empty state shows it
            _colorsDisabled {},
            _colorsHover {},
            _colorsClicked {},
            _colorsSelected {};
        ColorState _colorState { ColorState::Enabled };
        f32 _width { 8.0f };
        bool _isShown { false };
    };
}