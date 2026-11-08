#pragma once
#include "IncludeHelper.hpp"
#include "FillStyle.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    class Text;
    class ShadowRectangle;

    /// @brief A drop shadow cast by a background, a border or a text: their shape, moved by a distance at an angle, grown by a
    ///        scale and blurred at its edges, drawn behind them. Its colors take every fill and gradient, one set for every color
    ///        state. Reached by GetDropShadow(): button.GetBackground().GetDropShadow().SetDistance(4.0f). Hidden until Show(true)
    class DropShadow final : public FillStyle
    {
    public:
        DropShadow();
        DropShadow(const DropShadow&) = delete;
        DropShadow(DropShadow&&) = delete;
        ~DropShadow() = default;
        auto operator =(const DropShadow&) = delete;
        auto operator =(DropShadow&&) = delete;

        /// @brief Shows or hides the shadow. Hidden by default
        void Show(const bool show);

        /// @brief Returns whether the shadow is shown
        bool IsShown() const;

        /// @brief Sets the direction the shadow falls in, in degrees: 0 to the right, counterclockwise as the gradients turn, so
        ///        315 (the default) falls to the bottom right
        void SetDegree(const f32 degree);
        f32 GetDegree() const;

        /// @brief Sets how far the shadow falls from what casts it, in pixels. 4 by default
        void SetDistance(const f32 distance);
        f32 GetDistance() const;

        /// @brief Sets how wide the shadow's edges fade, in pixels, half inside its shape and half outside. 0 gives a hard edge.
        ///        8 by default. A text renders its shadow's glyphs once per blur, so a text's blur is best not changed every frame
        void SetBlur(const f32 blur);
        f32 GetBlur() const;

        /// @brief Sets how much bigger than what casts it the shadow is, around its center. 1 (the default) is the same size
        void SetScale(const f32 scale);
        f32 GetScale() const;

        /// @brief Sets the shadow's colors. Black at half opacity by default
        void SetColors(std::span<const glm::vec4> colors);

        /// @brief Returns the shadow's colors
        const std::vector<glm::vec4>& GetColors() const;

        using FillStyle::SetFillType;
        using FillStyle::SetGradientOffset;
        using FillStyle::SetGradientAnimationSpeed;
        using FillStyle::SetGradientDegree;
        using FillStyle::SetGradientRotationSpeed;
        using FillStyle::GetGradientOffset;
        using FillStyle::GetGradientDegree;

    //Called by what casts it
        /// @brief Returns how far the shadow is moved, in pixels with y up, from the degree and the distance
        glm::vec2 GetOffset() const;

    private:
        friend class Text; //draws it.
        friend class ShadowRectangle; //draws it.

        std::vector<glm::vec4> _shadowColors { glm::vec4(0.0f, 0.0f, 0.0f, 0.5f) }; //never empty. The fill keeps a span over it
        f32
            _degree { 315.0f },
            _distance { 4.0f },
            _blur { 8.0f },
            _scale { 1.0f };
        bool _isShown { false };
    };
}