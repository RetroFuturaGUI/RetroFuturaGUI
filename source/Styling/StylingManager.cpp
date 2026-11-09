#ifdef RETROFUTURAGUI_MODULES
module;
#include "ModuleExternals.hpp"
module RetroFuturaGUI;
#else
#include "StylingManager.hpp"
#endif

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

void RetroFuturaGUI::StylingManager::markChanged()
{
    ++_generation;
}

const RetroFuturaGUI::Flavor* RetroFuturaGUI::StylingManager::findFlavor(const u32 flavorHash)
{
    const auto found { _flavors.find(flavorHash) };
    return _flavors.end() == found ? nullptr : found->second.get();
}