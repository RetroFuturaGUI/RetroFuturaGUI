#include "ItemTextStyle.hpp"
#include <array>

RetroFuturaGUI::ItemTextStyle::ItemTextStyle()
{
    _colors.SetBuiltIn(std::array { glm::vec4(1.0f) }, ColorState::Enabled);
    _colors.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Disabled);
    _colors.SetBuiltIn(std::array { glm::vec4(1.0f) }, ColorState::Hover);
    _colors.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Clicked);
}

void RetroFuturaGUI::ItemTextStyle::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _colors.SetOwn(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::ItemTextStyle::GetColors(const ColorState state) const
{
    return _colors.GetOwn(state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::ItemTextStyle::GetShownColors(const ColorState state) const
{
    return _colors.GetShown(state, getFlavors(), isStyled(), getStylePart());
}

void RetroFuturaGUI::ItemTextStyle::restyle()
{
}
