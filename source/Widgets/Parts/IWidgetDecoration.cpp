#include "IWidgetDecoration.hpp"
#include <utility>

RetroFuturaGUI::IWidgetDecoration::IWidgetDecoration(Projection* projection, std::function<void()> onCornerRadiiChange)
    : _projection(projection), _onCornerRadiiChange(std::move(onCornerRadiiChange))
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

const glm::vec4& RetroFuturaGUI::IWidgetDecoration::GetCornerRadii() const
{
   return _cornerRadii;
}

void RetroFuturaGUI::IWidgetDecoration::SetCornerRadii(const glm::vec4& radii)
{
   _cornerRadii = radii;
   setCornerRadii();

    if(!_onCornerRadiiChange)
        return;

    _onCornerRadiiChange();
}