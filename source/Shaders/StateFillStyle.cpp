#include "StateFillStyle.hpp"
#include <utility>

RetroFuturaGUI::StateFillStyle::StateFillStyle(const glm::vec4& enabledColor)
    : _colorsEnabled { enabledColor }
{
    applyColors();
}

void RetroFuturaGUI::StateFillStyle::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Enabled == state && colors.empty())
        return; //every empty state shows Enabled, so it always holds a color.

    colorsOf(state).assign(colors.begin(), colors.end());
    applyColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::StateFillStyle::GetColors(const ColorState state) const
{
    return colorsOf(state);
}

void RetroFuturaGUI::StateFillStyle::SetColorState(const ColorState state)
{
    if(state == _colorState)
        return;

    _colorState = state;
    applyColors();
}

std::vector<glm::vec4>& RetroFuturaGUI::StateFillStyle::colorsOf(const ColorState state)
{
    return const_cast<std::vector<glm::vec4>&>(std::as_const(*this).colorsOf(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::StateFillStyle::colorsOf(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Disabled:
            return _colorsDisabled;
        case ColorState::Hover:
            return _colorsHover;
        case ColorState::Clicked:
            return _colorsClicked;
        case ColorState::Selected:
            return _colorsSelected;
        default: //Enabled
            return _colorsEnabled;
    }
}

void RetroFuturaGUI::StateFillStyle::applyColors()
{
    const std::vector<glm::vec4>& colors { colorsOf(_colorState) };
    FillStyle::SetColors(colors.empty() ? _colorsEnabled : colors);
}