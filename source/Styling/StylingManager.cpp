#ifdef RETROFUTURAGUI_MODULES
module;
#include "ModuleExternals.hpp"
module RetroFuturaGUI;
#else
#include "StylingManager.hpp"
#include <format>
#endif

RetroFuturaGUI::FlavorScope::FlavorScope(const StyleScope scope)
    : _scope(scope)
{
}

RetroFuturaGUI::Flavor& RetroFuturaGUI::FlavorScope::GetEntry(const WidgetTypeID widgetType, const StyleAccessor accessor) const
{
    return StylingManager::GetFlavor(StylingManager::getScopeEntryName(_scope, widgetType, accessor));
}

bool RetroFuturaGUI::FlavorScope::HasEntry(const WidgetTypeID widgetType, const StyleAccessor accessor) const
{
    return StylingManager::HasFlavor(StylingManager::getScopeEntryName(_scope, widgetType, accessor));
}

void RetroFuturaGUI::FlavorScope::RemoveEntry(const WidgetTypeID widgetType, const StyleAccessor accessor) const
{
    StylingManager::RemoveFlavor(StylingManager::getScopeEntryName(_scope, widgetType, accessor));
}

RetroFuturaGUI::FlavorScope RetroFuturaGUI::StylingManager::GetScope(const StyleScope scope)
{
    return FlavorScope(scope);
}

std::string RetroFuturaGUI::StylingManager::getScopeEntryName(const StyleScope scope, const WidgetTypeID widgetType, const StyleAccessor accessor)
{
    //Angle brackets, as no flavor name an application gives has them.
    return std::format("<{}>{}:{}", static_cast<u32>(scope), static_cast<i32>(widgetType), static_cast<u32>(accessor));
}

void RetroFuturaGUI::StylingManager::composeFlavorHashes(const StyleScope scope, const WidgetTypeID widgetType, const StyleAccessor accessors, std::span<const std::string> ownFlavors, std::vector<u32>& hashes)
{
    hashes.clear();

    //Each scope's accessor entries come before its plain one, the popup type's scope before the shared one.
    const auto addScope = [&hashes, widgetType, accessors](const StyleScope added)
    {
        for(const StyleAccessor accessor : { StyleAccessor::On, StyleAccessor::Invalid, StyleAccessor::Icon, StyleAccessor::Title })
        {
            if(HasStyleAccessor(accessors, accessor))
                hashes.push_back(GetFlavorHash(getScopeEntryName(added, widgetType, accessor)));
        }

        hashes.push_back(GetFlavorHash(getScopeEntryName(added, widgetType, StyleAccessor::None)));
    };

    if(StyleScope::None != scope && StyleScope::Popup != scope)
        addScope(scope);

    if(StyleScope::None != scope)
        addScope(StyleScope::Popup);

    for(const std::string& name : ownFlavors)
        hashes.push_back(GetFlavorHash(name));
}

RetroFuturaGUI::Flavor& RetroFuturaGUI::StylingManager::GetFlavor(std::string_view name)
{
    std::unique_ptr<Flavor>& flavor { _flavors[GetFlavorHash(name)] };

    if(!flavor)
    {
        flavor = std::make_unique<Flavor>();
        markChanged(); //widgets that named it before it existed take it now
    }

    return *flavor;
}

RetroFuturaGUI::Flavor& RetroFuturaGUI::StylingManager::GetDefaultFlavor()
{
    return GetFlavor("default");
}

bool RetroFuturaGUI::StylingManager::HasFlavor(std::string_view name)
{
    return _flavors.contains(GetFlavorHash(name));
}

void RetroFuturaGUI::StylingManager::RemoveFlavor(std::string_view name)
{
    const u32 flavorHash { GetFlavorHash(name) };

    if(GetFlavorHash("default") == flavorHash)
        return;

    if(0 == _flavors.erase(flavorHash))
        return;

    markChanged();
}

u32 RetroFuturaGUI::StylingManager::GetFlavorHash(std::string_view name)
{
    if(!name.empty() && '.' == name.front())
        name.remove_prefix(1);

    return Fnv1aHash(name);
}

u32 RetroFuturaGUI::StylingManager::GetGeneration()
{
    return _generation;
}

const std::vector<glm::vec4>* RetroFuturaGUI::StylingManager::FindColors(std::span<const u32> flavorHashes, const StylePart part, const ColorState state)
{
    return FindFirst<const std::vector<glm::vec4>*>(flavorHashes, part, [state](const FlavorPart& style) -> const std::vector<glm::vec4>*
    {
        const std::vector<glm::vec4>& colors { style.GetColors(state) };
        return colors.empty() ? nullptr : &colors;
    });
}

std::optional<glm::vec4> RetroFuturaGUI::StylingManager::FindCornerRadii(std::span<const u32> flavorHashes, const StylePart part)
{
    return FindFirst<std::optional<glm::vec4>>(flavorHashes, part, [](const FlavorPart& style) { return style.GetCornerRadii(); });
}

std::optional<RetroFuturaGUI::CutCorners> RetroFuturaGUI::StylingManager::FindCutCorners(std::span<const u32> flavorHashes, const StylePart part)
{
    return FindFirst<std::optional<CutCorners>>(flavorHashes, part, [](const FlavorPart& style) { return style.GetCutCorners(); });
}

std::optional<f32> RetroFuturaGUI::StylingManager::FindWidth(std::span<const u32> flavorHashes, const StylePart part)
{
    return FindFirst<std::optional<f32>>(flavorHashes, part, [](const FlavorPart& style) { return style.GetWidth(); });
}

void RetroFuturaGUI::StylingManager::ResolveTransitions(const TransitionSettings& own, std::span<const u32> flavorHashes, TransitionSettings& shown)
{
    for(const TransitionProperty property : Transitions::AllProperties)
    {
        const std::optional<Transition>& ownTransition { Transitions::GetOverride(own, property) };
        const std::optional<Transition> transition { ownTransition ? ownTransition : FindFirst<std::optional<Transition>>(flavorHashes, [property](const Flavor& flavor) -> const std::optional<Transition>& { return Transitions::GetOverride(flavor.GetTransitions(), property); }) };

        if(transition)
            Transitions::Override(shown, property, *transition);
        else
            Transitions::Reset(shown, property);
    }
}

void RetroFuturaGUI::StylingManager::markChanged()
{
    ++_generation;
}

const RetroFuturaGUI::Flavor* RetroFuturaGUI::StylingManager::findFlavor(const u32 flavorHash)
{
    const auto found { _flavors.find(flavorHash) };
    return _flavors.end() == found ? nullptr : found->second.get();
}