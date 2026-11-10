#include "Indicator.hpp"
#include <array>
#include <utility>

RetroFuturaGUI::Indicator::Indicator(Projection* projection, IndicatorHooks hooks)
    : _background(projection), _border(projection), _onLayoutChange(std::move(hooks._OnLayoutChange))
{
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) }, ColorState::Enabled);
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) }, ColorState::Clicked);
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) }, ColorState::Hover);
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Disabled);

    for(const ColorState state : { ColorState::Enabled, ColorState::Clicked, ColorState::Hover, ColorState::Disabled })
        _border.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(1.0f) }, state);
}

void RetroFuturaGUI::Indicator::Show(const bool show)
{
    _isShown = show;
    notifyLayoutChange();
}

bool RetroFuturaGUI::Indicator::IsShown() const
{
    return _isShown;
}

void RetroFuturaGUI::Indicator::SetType(const IndicatorType type)
{
    _type = type;
    applyType();
}

RetroFuturaGUI::IndicatorType RetroFuturaGUI::Indicator::GetType() const
{
    return _type;
}

void RetroFuturaGUI::Indicator::SetSize(const glm::vec2& size, const ElementSizing sizingMode)
{
    _size = size;
    _sizingMode = sizingMode;
    notifyLayoutChange();
}

RetroFuturaGUI::Background& RetroFuturaGUI::Indicator::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::Indicator::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::Indicator::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::Indicator::GetBorder() const
{
    return _border;
}

void RetroFuturaGUI::Indicator::Draw()
{
    if(!_isShown)
        return;

    _background.Draw();
    _border.Draw();
}

void RetroFuturaGUI::Indicator::Fit(const glm::vec2& space)
{
    const glm::vec2 size { ElementSizing::Percent == _sizingMode ? space * _size * 0.01f : glm::min(_size, space) };
    _background.SetSize(size);
    _border.SetSize(size);
    applyType();
}

void RetroFuturaGUI::Indicator::Place(const glm::vec3& position, const glm::vec3& rotation)
{
    _background.SetPosition(position);
    _background.SetRotation(rotation);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
    _border.SetRotation(rotation);
}

glm::vec2 RetroFuturaGUI::Indicator::GetDrawnSize() const
{
    return _background.GetSize();
}

glm::vec3 RetroFuturaGUI::Indicator::GetPosition() const
{
    return _background.GetPosition();
}

glm::vec3 RetroFuturaGUI::Indicator::GetRotation() const
{
    return _background.GetRotation();
}

void RetroFuturaGUI::Indicator::SetColorState(const ColorState state)
{
    _background.SetColorState(state);
    _border.SetColorState(state);
}

void RetroFuturaGUI::Indicator::SetTransitionSettings(const TransitionSettings* settings)
{
    _background.SetTransitionSettings(settings);
    _border.SetTransitionSettings(settings);
}

void RetroFuturaGUI::Indicator::applyType()
{
    if(IndicatorType::None == _type)
        return;

    const glm::vec4 radii { IndicatorType::Circle == _type ? _background.GetSize().x * 0.5f : 0.0f };
    _background.SetBuiltInCornerRadii(StyleKey {}, radii);
    _border.SetBuiltInCornerRadii(StyleKey {}, radii);
}

void RetroFuturaGUI::Indicator::notifyLayoutChange()
{
    if(!_onLayoutChange)
        return;

    _onLayoutChange();
}