#pragma once
#include "IncludeHelper.hpp"
#include "Flavor.hpp"
#include <memory>
#include <optional>
#include <span>
#include <string>
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
        friend class Indicator; //the parts that give the parts inside them their built-in look
        friend class DropDownList;
        friend class IconButton;
        friend class HoverText;
        friend class ColorPickerIndicator;
        friend class CellCheckBox;
        friend class Window;
        friend class DialogKit; //the library's own popups, whose widgets' look flavors override
        StyleKey() = default;
    };

    /// @brief Where a widget lies, for the scopes styling the widgets inside the library's popups. A widget in a popup takes
    ///        what its popup type's scope sets for its widget type, then what the shared Popup scope sets, before its own
    ///        flavors: StylingManager::GetScope(StyleScope::File).GetEntry(WidgetTypeID::Button).GetBorder().SetColors(...)
    RFG_EXPORT enum class StyleScope : u32
    {
        None, //outside every popup: the widget's flavors only.
        Popup, //every popup, after the popup type's own scope. A popup the application builds (Popup<Ts...>) takes only this one
        Message,
        Warning,
        Error,
        Value,
        File //the FileExplorer and its prompts.
    };

    /// @brief What a scope entry is for beyond the widget type, as a mask, like CSS pseudo-classes: <FileExplorer><Button:on>.
    ///        A widget takes the entries of every accessor it has, in this order, before its type's plain entry. States: On (a
    ///        toggle that is on), Invalid (a field holding no valid value). Roles: Icon, Title (a popup's icon and its bar's title)
    RFG_EXPORT enum class StyleAccessor : u32
    {
        None = 0,
        On = 1,
        Invalid = 1 << 1,
        Icon = 1 << 2,
        Title = 1 << 3
    };

    RFG_EXPORT constexpr StyleAccessor operator |(const StyleAccessor left, const StyleAccessor right)
    {
        return static_cast<StyleAccessor>(static_cast<u32>(left) | static_cast<u32>(right));
    }

    RFG_EXPORT constexpr bool HasStyleAccessor(const StyleAccessor accessors, const StyleAccessor accessor)
    {
        return 0 != (static_cast<u32>(accessors) & static_cast<u32>(accessor));
    }

    /// @brief One scope's entries: a flavor per widget type and accessor, which the widgets of that type inside the scope take
    ///        before their own flavors. StylingManager::GetScope hands one out.
    RFG_EXPORT class FlavorScope final
    {
    public:
        /// @brief The entry for the widget type with one accessor (or none), made the first time it is asked for. Everything a
        ///        flavor sets works here: <FileExplorer><Button><Border colors:hover=...> is
        ///        GetScope(StyleScope::File).GetEntry(WidgetTypeID::Button).GetBorder().SetColors(..., ColorState::Hover)
        Flavor& GetEntry(const WidgetTypeID widgetType, const StyleAccessor accessor = StyleAccessor::None) const;

        /// @brief Whether the entry exists.
        bool HasEntry(const WidgetTypeID widgetType, const StyleAccessor accessor = StyleAccessor::None) const;

        /// @brief Removes the entry, so the widgets go on to the next scope and their flavors
        void RemoveEntry(const WidgetTypeID widgetType, const StyleAccessor accessor = StyleAccessor::None) const;

    private:
        friend class StylingManager;
        FlavorScope(const StyleScope scope);

        StyleScope _scope { StyleScope::None };
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

        /// @brief Returns a scope's entries, which style the widgets inside the library's popups by widget type before their
        ///        flavors (StyleScope)
        static FlavorScope GetScope(const StyleScope scope);

        /// @brief Returns a number that changes whenever any flavor changes, so a widget's part knows to take its values again
        static u32 GetGeneration();

    //Called by the widgets' parts: the first of the flavors, then .default, that sets the value
        static const std::vector<glm::vec4>* FindColors(std::span<const u32> flavorHashes, const StylePart part, const ColorState state);
        static std::optional<glm::vec4> FindCornerRadii(std::span<const u32> flavorHashes, const StylePart part);
        static std::optional<CutCorners> FindCutCorners(std::span<const u32> flavorHashes, const StylePart part);
        static std::optional<f32> FindWidth(std::span<const u32> flavorHashes, const StylePart part);

        /// @brief Works out the transitions shown per property: the own one, else the first of the flavors' (then .default's). A
        ///        property none of them sets follows the global default
        static void ResolveTransitions(const TransitionSettings& own, std::span<const u32> flavorHashes, TransitionSettings& shown);

        /// @brief The first of the flavors, then .default, whose part style gives a value through pick (an optional or a
        ///        pointer that is empty when the style leaves it unset), or an empty value when none does
        template<typename Value, typename Pick> static Value FindFirst(std::span<const u32> flavorHashes, const StylePart part, Pick pick)
        {
            return FindFirst<Value>(flavorHashes, [part, &pick](const Flavor& flavor) -> Value
            {
                const FlavorPart* style { flavor.FindPart(part) };

                if(!style)
                    return Value {}; //the flavor sets nothing for this part inside the widget

                return pick(*style);
            });
        }

        /// @brief The first of the flavors, then .default, that gives a value through pick, for what a flavor sets on the widget
        ///        itself (its transitions, its border gap), or an empty value when none does
        template<typename Value, typename Pick> static Value FindFirst(std::span<const u32> flavorHashes, Pick pick)
        {
            for(const u32 flavorHash : flavorHashes)
            {
                const Flavor* flavor { findFlavor(flavorHash) };

                if(!flavor)
                    continue; //named but not made, or removed

                if(const Value value { pick(*flavor) })
                    return value;
            }

            const Flavor* defaultFlavor { findFlavor(GetFlavorHash("default")) };

            if(!defaultFlavor)
                return Value {};

            return pick(*defaultFlavor);
        }

    private:
        friend class FlavorScope; //keeps its entries as flavors under reserved names
        friend class IWidget; //composes its flavors with its scope's entries
        friend class Window;
        friend class DialogKit; //measures its popups from the fonts their scopes give
        friend class FlavorPart; //tells of its changes
        friend class Flavor;
        friend class FlavorTrackStyle;
        friend class FlavorCellStyle;
        friend class FlavorHeaderStyle;
        friend class FlavorEffect;

        /// @brief Moves the generation on: a flavor changed, came or went
        static void markChanged();

        /// @brief The flavor kept under the hash, or nullptr.
        static const Flavor* findFlavor(const u32 flavorHash);

        /// @brief The reserved name a scope entry is kept under. No flavor name an application gives looks like it
        static std::string getScopeEntryName(const StyleScope scope, const WidgetTypeID widgetType, const StyleAccessor accessor);

        /// @brief The flavors a widget or window reads: inside a popup, its popup type's entries for its type (one per accessor
        ///        it has, then the plain one), then the shared Popup scope's likewise, then its own flavors in order
        static void composeFlavorHashes(const StyleScope scope, const WidgetTypeID widgetType, const StyleAccessor accessors, std::span<const std::string> ownFlavors, std::vector<u32>& hashes);

        static inline std::unordered_map<u32, std::unique_ptr<Flavor>> _flavors {};
        static inline u32 _generation { 1 }; //parts start at 0, so they take their values on their first draw
    };
}