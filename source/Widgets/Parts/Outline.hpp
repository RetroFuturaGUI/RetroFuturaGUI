#pragma once
#include "IncludeHelper.hpp"
#include "ShaderLayers.hpp"
#include "Text.hpp"
#include <array>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The outline around every glyph of a widget's text. The WidgetText owns it and hands it out through GetOutline(), so
    ///        it is styled as button.GetText().GetOutline().SetWidth(...). Its width, colors and fill show its own value when
    ///        set, else the text's flavors' (FlavorPart::GetOutline), else the outline's default
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

        /// @brief Sets how far the outline reaches out from every glyph, in pixels, over every flavor. 0, the default, draws no outline
        void SetWidth(const f32 width);

        /// @brief Returns how far the outline reaches out from every glyph (in pixels)
        f32 GetWidth() const;

        /// @brief Sets the colors shown in the given state, which win over every flavor. More than one color makes the fill a
        ///        gradient. No colors leave the state to the flavors and the default (black); a state none of them gives colors,
        ///        such as Selected, shows the Enabled ones
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set on the outline itself for the given state, empty when unset
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

        /// @brief Leaves a setting to the text's flavors and the outline's default (colors: set none instead). The width, the
        ///        fill type and the gradient settings apply; the other effect settings are ignored
        void ResetSetting(const EffectSetting setting);

        /// @brief Sets the outline's own image, stretched across the whole text behind its fill, whose alpha decides how much of it shows. 0 shows no image
        void SetImageTextureID(const u32 textureID);

        /// @brief Sets how far the image stays clear of the text's edges per side (in pixels)
        void SetImagePadding(const f32 padding);

        /// @brief Sets how the image is laid across the text. Stretched, the default, or keeping its proportions
        void SetImageFit(const ImageFit fit);

        /// @brief Shows the colors of the given state. Called by the WidgetText when its widget is hovered, pressed, selected, enabled or disabled
        void SetColorState(const ColorState state);

    private:
        friend class WidgetText; //hands it the text's flavors.

        /// @brief Has the outline take unset values from the text's flavors, and takes them now
        void followFlavors(const std::vector<u32>* flavors);

        /// @brief Takes every setting through the layers again.
        void restyle();

        /// @brief The colors shown in a state: the outline's own, else its flavors', else its default; else the Enabled ones
        const std::vector<glm::vec4>& getShownColors(const ColorState state) const;

        /// @brief Hands the width and the fill settings that changed through the layers to the drawn outline
        void resolveSettings();

        /// @brief The text's flavors, empty for a text no flavor styles
        std::span<const u32> getFlavors() const;

        Text& _text;
        std::array<std::vector<glm::vec4>, 5>
            _ownColors {}, //per ColorState, in its order
            _builtInColors {};
        ColorState _colorState { ColorState::Enabled };
        const std::vector<u32>* _flavors { nullptr }; //the widget's flavors, kept by the widget. nullptr for a text no flavor styles
        LayeredSetting<f32> _widthLayer {};
        LayeredSetting<FillType> _fillTypeLayer {};
        LayeredSetting<f32>
            _gradientDegreeLayer {},
            _gradientOffsetLayer {},
            _gradientAnimationSpeedLayer {},
            _gradientRotationSpeedLayer {};
    };
}