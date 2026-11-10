#pragma once
#include "IncludeHelper.hpp"
#include "SvgTexture.hpp"
#include "StyledPart.hpp"
#include "ShaderLayers.hpp"
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief An icon's colors per state, given when the widget builds it
    struct IconColors
    {
        std::vector<glm::vec4>
            _Enabled {},
            _Disabled {},
            _Clicked {},
            _Hover {},
            _Selected {};
    };

    /// @brief An SVG icon a widget draws, such as a checkmark or a drop-down arrow, with one of its paths colored per state. The
    ///        widget owns it and hands it out through a getter, so it is styled as checkBox.GetCheckmark().SetColors(...)
    class Icon final : public IStyledPart
    {
    public:
        /// @param svg The icon's own SVG, from ResourceManager. Without one the icon draws nothing
        /// @param pathName The named path in the SVG that takes the colors
        /// @param style How the colors are drawn: the fill type and the gradient settings. Its colors are not used
        /// @param colors The colors per state: the icon's built-in look, which its widget's flavors and its own colors override
        Icon(std::unique_ptr<SvgTexture> svg, std::string_view pathName, const SvgPathFill& style, IconColors colors);
        Icon() = delete;
        Icon(const Icon&) = delete;
        Icon(Icon&&) = delete;
        ~Icon() = default;
        auto operator =(const Icon&) = delete;
        auto operator =(Icon&&) = delete;

        /// @brief Sets the colors shown in the given state, over every flavor. One for a solid fill, the stops for a gradient. No
        ///        colors leave the state to the flavors and the built-in colors; a state none of them sets shows the Enabled colors
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set on the icon itself for the given state, empty when unset
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the fill type (solid, linear/radial/huestar gradient)
        void SetFillType(const FillType fillType);

        /// @brief Sets the angle of the linear gradient, in degrees
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the offset applied to the gradient's start position
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the gradient animates over time
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the speed at which the gradient rotates over time
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Leaves a fill setting to the widget's flavors and the built-in look (colors: set none instead)
        void ResetSetting(const StyleSetting setting);

    //Called by the owning widget
        /// @brief Draws the icon
        void Draw();

        /// @brief Sizes the icon
        void SetSize(const glm::vec2& size);

        /// @brief Places the icon
        void SetPosition(const glm::vec3& position);

        /// @brief Turns the icon
        void SetRotation(const glm::vec3& rotation);

        /// @brief Returns the SVG's width over its height, 0 without an SVG
        f32 GetAspectRatio() const;

        /// @brief Renders the SVG again at the given resolution, so it stays sharp at the size it is drawn at
        void Rasterize(const glm::i32vec2& resolution);

        /// @brief Shows the colors of the given state
        void SetColorState(const ColorState state);

    private:
        void restyle() override;

        /// @brief Hands the path the colors of the current state and the style. Until the first call the SVG keeps its own colors
        void applyFill();

        std::unique_ptr<SvgTexture> _svg { nullptr };
        std::string _pathName {};
        SvgPathFill
            _fill {}, //what the path shows.
            _builtInFill {};
        LayeredColors _colors {};
        LayeredSetting<FillType> _fillTypeLayer {};
        LayeredSetting<f32>
            _gradientDegreeLayer {},
            _gradientOffsetLayer {},
            _gradientAnimationSpeedLayer {},
            _gradientRotationSpeedLayer {};
        ColorState _colorState { ColorState::Enabled };
        bool _isFilled { false }; //the path shows the icon's colors, not the SVG's own
    };
}