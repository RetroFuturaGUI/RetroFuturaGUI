#pragma once
#include "IncludeHelper.hpp"
#include "Text.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The outline around every glyph of a widget's text. The WidgetText owns it and hands it out through GetOutline(), so
    ///        it is styled as button.GetText().GetOutline().SetWidth(...)
    class Outline final
    {
    public:
        /// @param text The drawn text this outline goes around
        explicit Outline(Text& text);
        Outline() = delete;
        Outline(const Outline&) = delete;
        Outline(Outline&&) = delete;
        ~Outline() = default;
        auto operator =(const Outline&) = delete;
        auto operator =(Outline&&) = delete;

        /// @brief Sets how far the outline reaches out from every glyph, in pixels. 0, the default, draws no outline
        void SetWidth(const f32 width);

        /// @brief Returns how far the outline reaches out from every glyph (in pixels)
        f32 GetWidth() const;

        /// @brief Sets the colors shown in the given state (black by default). More than one color makes the fill a
        ///        gradient. Selected colors are optional: without any, the Selected state shows the Enabled colors
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors for the given state
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the fill type
        void SetFillType(const FillType fillType);

        /// @brief Sets the offset applied to the gradient's start position
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the gradient animates
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle (degrees) of the linear shift
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the gradient rotates
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the outline's own image, stretched across the whole text behind its fill, whose alpha decides how much of it shows. 0 shows no image
        void SetImageTextureID(const u32 textureID);

        /// @brief Sets how far the image stays clear of the text's edges per side (in pixels)
        void SetImagePadding(const f32 padding);

        /// @brief Sets how the image is laid across the text. Stretched, the default, or keeping its proportions
        void SetImageFit(const ImageFit fit);

        /// @brief Shows the colors of the given state. Called by the WidgetText when its widget is hovered, pressed, selected, enabled or disabled
        void SetColorState(const ColorState state);

    private:
        std::vector<glm::vec4>& colorsOf(const ColorState state);
        const std::vector<glm::vec4>& colorsOf(const ColorState state) const;

        /// @brief The colors a state shows. Its own, or the Enabled ones for a Selected state that has none
        const std::vector<glm::vec4>& shownColorsOf(const ColorState state) const;

        Text& _text;
        std::vector<glm::vec4>
            _colorsEnabled { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
            _colorsDisabled { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
            _colorsClicked { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
            _colorsHover { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
            _colorsSelected {};
        ColorState _colorState { ColorState::Enabled };
    };
}