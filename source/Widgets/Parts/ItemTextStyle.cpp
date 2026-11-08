#include "ItemTextStyle.hpp"

void RetroFuturaGUI::ItemTextStyle::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(state).assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::ItemTextStyle::GetColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Disabled:
            return _disabled;
        case ColorState::Hover:
            return _hover;
        case ColorState::Clicked:
            return _clicked;
        case ColorState::Selected:
            return _selected;
        default: // Enabled
            return _enabled;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::ItemTextStyle::GetShownColors(const ColorState state) const
{
    if(ColorState::Selected == state && _selected.empty())
        return _enabled;

    return GetColors(state);
}

std::vector<glm::vec4>& RetroFuturaGUI::ItemTextStyle::colorsOf(const ColorState state)
{
    switch(state)
    {
        case ColorState::Disabled:
            return _disabled;
        case ColorState::Hover:
            return _hover;
        case ColorState::Clicked:
            return _clicked;
        case ColorState::Selected:
            return _selected;
        default: // Enabled
            return _enabled;
    }
}
