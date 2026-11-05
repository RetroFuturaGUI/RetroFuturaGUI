#include "IWidgetDecoration.hpp"

RetroFuturaGUI::IWidgetDecoration::IWidgetDecoration(Projection* projection)
    : _projection(projection)
{

}

void RetroFuturaGUI::IWidgetDecoration::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Disabled:
           _colorsDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
           _colorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
           _colorsHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Selected:
           _colorsSelected.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
           _colorsEnabled.assign(colors.begin(), colors.end());
    }

    setColors();
}

void RetroFuturaGUI::IWidgetDecoration::SetColorState(const ColorState state)
{
    _colorState = state;
    setColors();
}