#include "IWidget.hpp"

RetroFuturaGUI::IWidget::IWidget(const IdentityParams& identity, const GeometryParams2D& geometry) 
    : _name(identity._Name), _parent(identity._Parent), _parentTypeID(identity._ParentTypeID), _parentWindow(identity._ParentWindow),
     _projection(const_cast<Projection&>(geometry._Projection)), _position(geometry._Position), _size(geometry._Size), _rotation(geometry._Rotation)
{
    
}

void RetroFuturaGUI::IWidget::Connect_OnEnable(const typename Signal<>::Slot & slot, const bool async)
{
    if (async)
        _onEnableAsync.Connect(slot);
    else
        _onEnable.Connect(slot);
}

void RetroFuturaGUI::IWidget::Connect_OnDisable(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onDisableAsync.Connect(slot);
    else
        _onDisable.Connect(slot);
}

void RetroFuturaGUI::IWidget::Disconnect_OnEnable(const typename Signal<>::Slot &slot)
{
    _onEnable.Disconnect(slot);
    _onEnableAsync.Disconnect(slot);
}

void RetroFuturaGUI::IWidget::Disconnect_OnDisable(const typename Signal<>::Slot &slot)
{
    _onDisable.Disconnect(slot);
    _onDisableAsync.Disconnect(slot);
}

void RetroFuturaGUI::IWidget::SetSize(const glm::vec2 &size)
{
    _size = size;
}

glm::vec2 RetroFuturaGUI::IWidget::GetSize() const
{
    return _size;
}

void RetroFuturaGUI::IWidget::SetPosition(const glm::vec2 &position)
{
    _position = position;
}

glm::vec2 RetroFuturaGUI::IWidget::GetPosition() const
{
    return _position;
}

void RetroFuturaGUI::IWidget::SetRotation(const f32 rotation)
{
    _rotation = rotation;
}

f32 RetroFuturaGUI::IWidget::GetRotation() const
{
    return _rotation;
}

std::span<glm::vec4> RetroFuturaGUI::IWidget::GetBackgroundColors()
{
    return std::span<glm::vec4>(_backgroundColors.get(), _colorCount);
}

void RetroFuturaGUI::IWidget::SetBackgroundColors(std::span<glm::vec4> backgroundColors)
{
    _colorCount = static_cast<i32>(backgroundColors.size());
    _backgroundColorFillType = _colorCount > 1 ? FillType::GRADIENT : FillType::SOLID;

    _backgroundColors = std::make_unique<glm::vec4[]>(_colorCount);

    for (uSize i = 0; i < backgroundColors.size(); ++i)
        _backgroundColors[i] = backgroundColors[i];
}

std::string_view RetroFuturaGUI::IWidget::GetName() const
{
    return _name;
}

void RetroFuturaGUI::IWidget::SetName(std::string_view name)
{
    _name = name;
}

RetroFuturaGUI::WidgetTypeID RetroFuturaGUI::IWidget::GetWidgetTypeID() const
{
    return _widgetTypeID;
}

void RetroFuturaGUI::IWidget::SetEnabled(const bool enable, [[maybe_unused]] const bool emitSignal)
{
    _isEnabledFlag = enable;
}

bool RetroFuturaGUI::IWidget::IsEnabled() const
{
    return _isEnabledFlag;
}
