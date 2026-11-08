#pragma once
#include "IncludeHelper.hpp"
#include "FillStyle.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief A fill with colors per color state, for the effects a part draws around or over itself (DropShadow, Glow,
    ///        InnerGlow): they follow the state of the part they belong to. A state without colors of its own shows the
    ///        Enabled ones, so setting Enabled alone gives one look for every state
    class StateFillStyle : public FillStyle
    {
    public:
        StateFillStyle(const StateFillStyle&) = delete;
        StateFillStyle(StateFillStyle&&) = delete;
        auto operator =(const StateFillStyle&) = delete;
        auto operator =(StateFillStyle&&) = delete;

        /// @brief Sets the colors for a color state. A state without colors of its own shows the Enabled ones
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

    private:
        /// @brief The colors set for a state.
        std::vector<glm::vec4>& colorsOf(const ColorState state);
        const std::vector<glm::vec4>& colorsOf(const ColorState state) const;

        /// @brief Hands the colors of the state shown to the fill.
        void applyColors();

        std::vector<glm::vec4>
            _colorsEnabled {}, //never empty: every empty state shows it.
            _colorsDisabled {},
            _colorsHover {},
            _colorsClicked {},
            _colorsSelected {};
        ColorState _colorState { ColorState::Enabled };
    };
}