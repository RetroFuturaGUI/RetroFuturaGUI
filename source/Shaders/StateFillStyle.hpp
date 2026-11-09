#pragma once
#include "IncludeHelper.hpp"
#include "FillStyle.hpp"
#include "ShaderLayers.hpp"
#include <array>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief A fill with colors per color state, for the effects a part draws around or over itself (DropShadow, Glow,
    ///        InnerGlow): they follow the state of the part they belong to. Every setting shows the effect's own value when
    ///        set, else its part's flavors' (StylingManager), else the effect's default. A state none of them gives colors
    ///        shows the Enabled ones, so setting Enabled alone gives one look for every state
    class StateFillStyle : public FillStyle
    {
    public:
        StateFillStyle(const StateFillStyle&) = delete;
        StateFillStyle(StateFillStyle&&) = delete;
        auto operator =(const StateFillStyle&) = delete;
        auto operator =(StateFillStyle&&) = delete;

        /// @brief Sets the colors for a color state, which win over every flavor. No colors leave the state to the flavors
        ///        and the effect's default
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set on the effect itself for a color state, empty when unset
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Shows or hides the effect, over every flavor. Hidden by default
        void Show(const bool show);

        /// @brief Returns whether the effect is shown
        bool IsShown() const;

        /// @brief Set the fill, which win over every flavor: the fill type and the gradient's offset, speeds and angle
        void SetFillType(const FillType fillType);
        void SetGradientOffset(const f32 gradientOffset);
        void SetGradientAnimationSpeed(const f32 animationSpeed);
        void SetGradientDegree(const f32 degree);
        void SetGradientRotationSpeed(const f32 rotationSpeed);
        using FillStyle::GetGradientOffset;
        using FillStyle::GetGradientDegree;

        /// @brief Leaves a setting to the part's flavors and the effect's default (colors: set none instead)
        void ResetSetting(const EffectSetting setting);

    //Called by the part it belongs to
        /// @brief Shows the colors of the given state, the Enabled ones when the state has none
        void SetColorState(const ColorState state);

    protected:
        /// @param enabledColor The Enabled color until colors are set
        explicit StateFillStyle(const glm::vec4& enabledColor);
        ~StateFillStyle() = default;

        /// @brief Carries one of the effect's values (an angle, a distance, a blur, a width, a scale) to a new target over the
        ///        widget's Effects transition. Set before the first draw, or without transition settings, it shows at once
        void setEffectValue(TransitionValue<f32>& value, const f32 target);

        /// @brief How far an effect is moved, in pixels with y up, by an angle in degrees and a distance
        static glm::vec2 getOffset(const f32 degree, const f32 distance);

        /// @brief Which of a flavor part's effects this one takes its unset values from
        virtual const FlavorEffect& pickFlavor(const FlavorPart& part) const = 0;

        /// @brief Takes the effect's numbers through the layers, each with resolveNumber
        virtual void resolveNumbers() = 0;

        /// @brief Resets one of the effect's numbers; false for a setting it doesn't have
        virtual bool resetNumber(const EffectSetting setting) = 0;

        /// @brief Takes one number through the layers and moves the shown value to it when it changed
        /// @param get The flavor effect's getter for the number
        /// @param isNonNegative Whether the number can't go below 0 (a blur, a scale, a width)
        void resolveNumber(LayeredSetting<f32>& layer, TransitionValue<f32>& value, const f32 builtIn, const std::optional<f32>& (FlavorEffect::*get)() const, const bool isNonNegative);

    private:
        friend class Background; //hand their flavors to their effects.
        friend class Border;
        friend class WidgetText;

        /// @brief Has the effect take unset values from its part's flavors, and takes them now
        void followFlavors(const std::vector<u32>* flavors, const StylePart part);

        /// @brief Takes every setting through the layers again.
        void restyle();

        /// @brief The colors shown in a state: the effect's own, else its flavors', else its default; else the Enabled ones
        const std::vector<glm::vec4>& getShownColors(const ColorState state) const;

        /// @brief Hands the colors of the state shown to the fill.
        void applyColors();

        /// @brief Hands the fill settings that changed through the layers to the fill
        void resolveFill();

        /// @brief Takes whether the effect shows through the layers.
        void resolveShown();

        /// @brief The part's flavors, empty for an effect no flavor styles.
        std::span<const u32> getFlavors() const;

        std::array<std::vector<glm::vec4>, 5>
            _ownColors {}, //per ColorState, in its order.
            _builtInColors {};
        ColorState _colorState { ColorState::Enabled };
        const std::vector<u32>* _flavors { nullptr }; //the widget's flavors, kept by the widget. nullptr for a sub-part's effect
        StylePart _stylePart { StylePart::Background };
        LayeredSetting<bool> _shownLayer {};
        bool _isShown { false };
        LayeredSetting<FillType> _fillTypeLayer {};
        LayeredSetting<f32>
            _gradientDegreeLayer {},
            _gradientOffsetLayer {},
            _gradientAnimationSpeedLayer {},
            _gradientRotationSpeedLayer {};
    };
}
