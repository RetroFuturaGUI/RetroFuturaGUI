#pragma once
#include "IncludeHelper.hpp"
#include "StylingManager.hpp"
#include <optional>
#include <span>

namespace RetroFuturaGUI
{
    /// @brief A part's fill settings through the style layers: the part's own when set, else its widget's flavors'
    ///        and .default's, else the element's own defaults. An element is handed only a setting whose value changed, so a
    ///        flavor changing elsewhere doesn't restart a gradient that is turning or moving on. Kept by the parts
    RFG_EXPORT class FillLayers final
    {
    public:
        /// @brief The part's own settings, which win over every flavor. Setting one again hands it to the element again, so
        ///        setting the gradient's angle or offset restarts it from there
        void SetFillType(const FillType fillType) { _ownFillType = fillType; _shownFillType.reset(); }
        void SetGradientDegree(const f32 degree) { _ownGradientDegree = degree; _shownGradientDegree.reset(); }
        void SetGradientOffset(const f32 offset) { _ownGradientOffset = offset; _shownGradientOffset.reset(); }
        void SetGradientAnimationSpeed(const f32 speed) { _ownGradientAnimationSpeed = speed; _shownGradientAnimationSpeed.reset(); }
        void SetGradientRotationSpeed(const f32 speed) { _ownGradientRotationSpeed = speed; _shownGradientRotationSpeed.reset(); }

        /// @brief Leaves a setting to the widget's flavors, .default and the element's default
        void ResetFillType() { _ownFillType.reset(); }
        void ResetGradientDegree() { _ownGradientDegree.reset(); }
        void ResetGradientOffset() { _ownGradientOffset.reset(); }
        void ResetGradientAnimationSpeed() { _ownGradientAnimationSpeed.reset(); }
        void ResetGradientRotationSpeed() { _ownGradientRotationSpeed.reset(); }

        /// @brief Hands the element every setting whose value through the layers changed since it was last handed over
        /// @param isStyled Whether the part takes its widget's flavors at all: a sub-part goes from its own straight to the default
        template<typename Element> void Apply(Element& element, std::span<const u32> flavorHashes, const bool isStyled, const StylePart part)
        {
            applyOne(_ownFillType, _shownFillType, FillType::Solid, flavorHashes, isStyled, part,
                [](const FlavorPart& style) { return style.GetFillType(); }, [&element](const FillType value) { element.SetFillType(value); });
            applyOne(_ownGradientDegree, _shownGradientDegree, 45.0f, flavorHashes, isStyled, part,
                [](const FlavorPart& style) { return style.GetGradientDegree(); }, [&element](const f32 value) { element.SetGradientDegree(value); });
            applyOne(_ownGradientOffset, _shownGradientOffset, 0.0f, flavorHashes, isStyled, part,
                [](const FlavorPart& style) { return style.GetGradientOffset(); }, [&element](const f32 value) { element.SetGradientOffset(value); });
            applyOne(_ownGradientAnimationSpeed, _shownGradientAnimationSpeed, 0.018f, flavorHashes, isStyled, part,
                [](const FlavorPart& style) { return style.GetGradientAnimationSpeed(); }, [&element](const f32 value) { element.SetGradientAnimationSpeed(value); });
            applyOne(_ownGradientRotationSpeed, _shownGradientRotationSpeed, 1.2f, flavorHashes, isStyled, part,
                [](const FlavorPart& style) { return style.GetGradientRotationSpeed(); }, [&element](const f32 value) { element.SetGradientRotationSpeed(value); });
        }

    private:
        /// @brief Takes one setting through the layers and hands it on when it differs from what was handed on last
        template<typename Value, typename Pick, typename Set> static void applyOne(const std::optional<Value>& own, std::optional<Value>& shown, const Value builtIn,
            std::span<const u32> flavorHashes, const bool isStyled, const StylePart part, Pick pick, Set set)
        {
            const Value value { own ? *own : isStyled ? StylingManager::FindFirst<std::optional<Value>>(flavorHashes, part, pick).value_or(builtIn) : builtIn };

            if(shown && value == *shown)
                return;

            shown = value;
            set(value);
        }

        //The built-in values match FillStyle's defaults, so a part without settings shows what its element starts with
        std::optional<FillType>
            _ownFillType {},
            _shownFillType {};
        std::optional<f32>
            _ownGradientDegree {},
            _shownGradientDegree {},
            _ownGradientOffset {},
            _shownGradientOffset {},
            _ownGradientAnimationSpeed {},
            _shownGradientAnimationSpeed {},
            _ownGradientRotationSpeed {},
            _shownGradientRotationSpeed {};
    };
}