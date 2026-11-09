#pragma once
#include "IncludeHelper.hpp"
#include "IRectangle.hpp"
#include <array>
#include <optional>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The part of a widget a flavor's part style applies to.
    RFG_EXPORT enum class StylePart : u32
    {
        Background,
        Border,
        Text
    };

    /// @brief What a flavor sets for one part of the widgets that have it: colors per state, corner radii, cut corners and,
    ///        for a border, its width. Whatever it leaves unset comes from the widget's next flavor, then .default, then the
    ///        widget's built-in look. Whatever the widget sets itself wins over all of them. It also sets the fill. Its type
    ///        and the gradient's angle, offset and speeds. A change shows on every widget
    ///        with the flavor from its next draw. Reached by StylingManager::GetFlavor("primary").GetBackground()
    RFG_EXPORT class FlavorPart final
    {
    public:
        FlavorPart() = default;
        FlavorPart(const FlavorPart&) = delete;
        FlavorPart(FlavorPart&&) = delete;
        auto operator =(const FlavorPart&) = delete;
        auto operator =(FlavorPart&&) = delete;

        /// @brief Sets the colors for a color state. One for a solid fill, the stops for a gradient. No colors leave the
        ///        state unset, so it comes from further down
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set for a color state, empty when unset
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the corner radii, clockwise from the top left
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Leaves the corner radii unset
        void ResetCornerRadii();

        /// @brief Returns the corner radii, or nothing when unset
        const std::optional<glm::vec4>& GetCornerRadii() const;

        /// @brief Sets which corners are cut off straight instead of rounded
        void SetCutCorners(const CutCorners corners);

        /// @brief Leaves the cut corners unset
        void ResetCutCorners();

        /// @brief Returns the cut corners, or nothing when unset
        const std::optional<CutCorners>& GetCutCorners() const;

        /// @brief Sets a border's width in pixels. Backgrounds and texts have none
        void SetWidth(const f32 width);

        /// @brief Leaves the width unset
        void ResetWidth();

        /// @brief Returns the width, or nothing when unset
        const std::optional<f32>& GetWidth() const;

        /// @brief Sets the fill type (solid, linear, radial or conic shift)
        void SetFillType(const FillType fillType);

        /// @brief Leaves it unset
        void ResetFillType();

        /// @brief Returns it, or nothing when unset
        const std::optional<FillType>& GetFillType() const;

        /// @brief Sets the gradient's angle in degrees
        void SetGradientDegree(const f32 gradientDegree);

        /// @brief Leaves it unset
        void ResetGradientDegree();

        /// @brief Returns it, or nothing when unset
        const std::optional<f32>& GetGradientDegree() const;

        /// @brief Sets where the gradient's colors start, 0 to 1
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Leaves it unset
        void ResetGradientOffset();

        /// @brief Returns it, or nothing when unset
        const std::optional<f32>& GetGradientOffset() const;

        /// @brief Sets how fast the gradient's colors move on, in runs through all colors per second
        void SetGradientAnimationSpeed(const f32 gradientAnimationSpeed);

        /// @brief Leaves it unset
        void ResetGradientAnimationSpeed();

        /// @brief Returns it, or nothing when unset
        const std::optional<f32>& GetGradientAnimationSpeed() const;

        /// @brief Sets how fast the gradient turns, in degrees per second
        void SetGradientRotationSpeed(const f32 gradientRotationSpeed);

        /// @brief Leaves it unset
        void ResetGradientRotationSpeed();

        /// @brief Returns it, or nothing when unset
        const std::optional<f32>& GetGradientRotationSpeed() const;

    private:
        std::array<std::vector<glm::vec4>, 5> _colors {}; //per ColorState, in its order
        std::optional<glm::vec4> _cornerRadii {};
        std::optional<CutCorners> _cutCorners {};
        std::optional<f32> _width {};
        std::optional<FillType> _fillType {};
        std::optional<f32> _gradientDegree {};
        std::optional<f32> _gradientOffset {};
        std::optional<f32> _gradientAnimationSpeed {};
        std::optional<f32> _gradientRotationSpeed {};
    };

    /// @brief A named style rule, like a CSS class: what it sets for the background, the border and the text of every
    ///        widget that has it (IWidget::AddFlavor). Kept by the StylingManager
    RFG_EXPORT class Flavor final
    {
    public:
        Flavor() = default;
        Flavor(const Flavor&) = delete;
        Flavor(Flavor&&) = delete;
        auto operator =(const Flavor&) = delete;
        auto operator =(Flavor&&) = delete;

        /// @brief Returns what the flavor sets for its widgets' backgrounds
        FlavorPart& GetBackground();
        const FlavorPart& GetBackground() const;

        /// @brief Returns what the flavor sets for its widgets' borders
        FlavorPart& GetBorder();
        const FlavorPart& GetBorder() const;

        /// @brief Returns what the flavor sets for its widgets' texts
        FlavorPart& GetText();
        const FlavorPart& GetText() const;

        /// @brief Returns what the flavor sets for one of its widgets' parts
        const FlavorPart& GetPart(const StylePart part) const;

    private:
        FlavorPart
            _background {},
            _border {},
            _text {};
    };
}