#include "Glow.hpp"
#include <algorithm>
#include <utility>

RetroFuturaGUI::Glow::Glow()
{
    applyColors();
}

void RetroFuturaGUI::Glow::Show(const bool show)
{
    _isShown = show;
}

bool RetroFuturaGUI::Glow::IsShown() const
{
    return _isShown;
}

void RetroFuturaGUI::Glow::SetWidth(const f32 width)
{
    _width = std::max(width, 0.0f);
}

f32 RetroFuturaGUI::Glow::GetWidth() const
{
    return _width;
}

void RetroFuturaGUI::Glow::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Enabled == state && colors.empty())
        return; //every empty state shows Enabled, so it always holds a color

    colorsOf(state).assign(colors.begin(), colors.end());
    applyColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::Glow::GetColors(const ColorState state) const
{
    return colorsOf(state);
}

void RetroFuturaGUI::Glow::SetColorState(const ColorState state)
{
    if(state == _colorState)
        return;

    _colorState = state;
    applyColors();
}

std::vector<glm::vec4>& RetroFuturaGUI::Glow::colorsOf(const ColorState state)
{
    return const_cast<std::vector<glm::vec4>&>(std::as_const(*this).colorsOf(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::Glow::colorsOf(const ColorState state) const
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

void RetroFuturaGUI::Glow::applyColors()
{
    const std::vector<glm::vec4>& colors { colorsOf(_colorState) };
    FillStyle::SetColors(colors.empty() ? _colorsEnabled : colors);
}