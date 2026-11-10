#include "StyledPart.hpp"

void RetroFuturaGUI::IStyledPart::followFlavors()
{
    if(StylingManager::GetGeneration() == _styleGeneration)
        return;

    _styleGeneration = StylingManager::GetGeneration();
    restyle();
}

std::span<const u32> RetroFuturaGUI::IStyledPart::getFlavors() const
{
    if(!_flavors)
        return {};

    return *_flavors;
}

bool RetroFuturaGUI::IStyledPart::isStyled() const
{
    return nullptr != _flavors;
}

RetroFuturaGUI::StylePart RetroFuturaGUI::IStyledPart::getStylePart() const
{
    return _stylePart;
}

void RetroFuturaGUI::IStyledPart::setStyleSource(const std::vector<u32>* flavors, const StylePart part)
{
    _flavors = flavors;
    _stylePart = part;
    _styleGeneration = StylingManager::GetGeneration();
    restyle();
}

void RetroFuturaGUI::LayeredColors::SetOwn(std::span<const glm::vec4> colors, const ColorState state)
{
    _own[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::LayeredColors::GetOwn(const ColorState state) const
{
    return _own[static_cast<uSize>(state)];
}

void RetroFuturaGUI::LayeredColors::SetBuiltIn(std::span<const glm::vec4> colors, const ColorState state)
{
    _builtIn[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::LayeredColors::GetShown(const ColorState state, std::span<const u32> flavors, const bool isStyled, const StylePart part) const
{
    if(const std::vector<glm::vec4>* colors { Find(state, flavors, isStyled, part) })
        return *colors;

    if(ColorState::Enabled == state)
        return _builtIn[static_cast<uSize>(ColorState::Enabled)]; //empty: nothing gives Enabled colors.

    return GetShown(ColorState::Enabled, flavors, isStyled, part); //a state nothing sets looks like Enabled.
}

const std::vector<glm::vec4>* RetroFuturaGUI::LayeredColors::Find(const ColorState state, std::span<const u32> flavors, const bool isStyled, const StylePart part) const
{
    const std::vector<glm::vec4>& own { _own[static_cast<uSize>(state)] };

    if(!own.empty())
        return &own;

    if(const std::vector<glm::vec4>* styled { isStyled ? StylingManager::FindColors(flavors, part, state) : nullptr })
        return styled;

    const std::vector<glm::vec4>& builtIn { _builtIn[static_cast<uSize>(state)] };

    if(builtIn.empty())
        return nullptr;

    return &builtIn;
}
