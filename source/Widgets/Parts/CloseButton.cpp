#include "CloseButton.hpp"
#include <array>
#include <utility>

RetroFuturaGUI::CloseButton::CloseButton(Projection* projection, std::unique_ptr<SvgTexture> icon)
    : _background(projection),
      _icon
      {
          std::move(icon),
          "CloseIcon",
          SvgPathFill { ._FillType = FillType::Solid, ._Colors = {}, ._GradientDegree = 0.0f, ._GradientOffset = 0.0f, ._GradientAnimationSpeed = 0.0f, ._GradientRotationSpeed = 0.0f },
          IconColors { ._Enabled = { glm::vec4(0.85f, 0.85f, 0.9f, 1.0f) }, ._Disabled = {}, ._Clicked = {}, ._Hover = {}, ._Selected = {} }
      }
{
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(0.0f) }, ColorState::Enabled);
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(1.0f, 1.0f, 1.0f, 0.15f) }, ColorState::Hover);
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(1.0f, 1.0f, 1.0f, 0.25f) }, ColorState::Clicked);
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(0.0f) }, ColorState::Disabled);
    _icon.SetColorState(ColorState::Enabled); //the cross takes its color at once, not the SVG's own.
}

RetroFuturaGUI::Background& RetroFuturaGUI::CloseButton::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::CloseButton::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Icon& RetroFuturaGUI::CloseButton::GetIcon()
{
    return _icon;
}

const RetroFuturaGUI::Icon& RetroFuturaGUI::CloseButton::GetIcon() const
{
    return _icon;
}