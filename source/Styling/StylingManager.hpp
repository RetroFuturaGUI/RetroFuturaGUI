#pragma once
#include "IncludeHelper.hpp"
#include "Flavor.hpp"
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Lets a widget set its parts' built-in look, which every flavor and the widget's own settings override. Only
    ///        widgets can make one (IWidget::styleKey), so the built-in look isn't something an application sets
    RFG_EXPORT class StyleKey final
    {
    private:
        friend class IWidget;
        StyleKey() = default;
    };

    /// @brief Keeps the flavors, like a style sheet keeps CSS classes. A widget's part shows what the widget sets
    ///        itself. What it leaves unset comes from the first of its flavors that sets it (IWidget::AddFlavor), then
    ///        from .default, then from the widget's built-in look. Widgets don't own what they take from a flavor. A change
    ///        here shows on every widget with the flavor from its next draw. Used from the GUI thread only
    RFG_EXPORT class StylingManager final
    {
    public:
        StylingManager() = delete;

        /// @brief Returns the flavor with the name, made empty the first time it is asked for. The name is taken with or
        ///        without its leading dot. "primary" and ".primary" are the same flavor
        static Flavor& GetFlavor(std::string_view name);

        /// @brief Returns .default, which styles every widget's parts where neither the widget nor its flavors do. Empty
        ///        until it is given values, so widgets show their built-in look
        static Flavor& GetDefaultFlavor();

        /// @brief Whether a flavor with the name exists
        static bool HasFlavor(std::string_view name);

        /// @brief Removes the flavor. Its widgets keep its name and take their values from their other flavors. A flavor made
        ///        again under the name styles them again. .default can't be removed
        static void RemoveFlavor(std::string_view name);

        /// @brief Returns the hash a flavor is kept under. Fnv1aHash of its name without the leading dot
        static u32 GetFlavorHash(std::string_view name);

        /// @brief Returns a number that changes whenever any flavor changes, so a widget's part knows to take its values again
        static u32 GetGeneration();

    //Called by the widgets' parts: the first of the flavors, then .default, that sets the value
        static const std::vector<glm::vec4>* FindColors(std::span<const u32> flavorHashes, const StylePart part, const ColorState state);
        static std::optional<glm::vec4> FindCornerRadii(std::span<const u32> flavorHashes, const StylePart part);
        static std::optional<CutCorners> FindCutCorners(std::span<const u32> flavorHashes, const StylePart part);
        static std::optional<f32> FindWidth(std::span<const u32> flavorHashes, const StylePart part);

        /// @brief The first of the flavors, then .default, whose part style gives a value through pick (an optional or a
        ///        pointer that is empty when the style leaves it unset), or an empty value when none does
        template<typename Value, typename Pick> static Value FindFirst(std::span<const u32> flavorHashes, const StylePart part, Pick pick)
        {
            for(const u32 flavorHash : flavorHashes)
            {
                const Flavor* flavor { findFlavor(flavorHash) };

                if(!flavor)
                    continue; //named but not made, or removed

                if(const Value value { pick(flavor->GetPart(part)) })
                    return value;
            }

            const Flavor* defaultFlavor { findFlavor(GetFlavorHash("default")) };

            if(!defaultFlavor)
                return Value {};

            return pick(defaultFlavor->GetPart(part));
        }

    private:
        friend class FlavorPart; //tells of its changes
        friend class FlavorEffect;

        /// @brief Moves the generation on: a flavor changed, came or went
        static void markChanged();

        /// @brief The flavor kept under the hash, or nullptr.
        static const Flavor* findFlavor(const u32 flavorHash);

        static inline std::unordered_map<u32, std::unique_ptr<Flavor>> _flavors {};
        static inline u32 _generation { 1 }; //parts start at 0, so they take their values on their first draw
    };
}