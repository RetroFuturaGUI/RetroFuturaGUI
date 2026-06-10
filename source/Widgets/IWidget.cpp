#include "IWidget.hpp"

RetroFuturaGUI::IWidget::IWidget(const IdentityParams& identity, const GeometryParams3D& geometry) 
    : _name(identity._Name), _parent(identity._Parent), _parentTypeID(identity._ParentTypeID),
     _projection(const_cast<Projection&>(geometry._Projection)), _position(geometry._Position), _size(geometry._Size), _rotation(geometry._Rotation)
{
    _parentWindow = identity._ParentWindow;
}

void RetroFuturaGUI::IWidget::Connect_OnEnable(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onEnableAsync.Connect(slot);
    else
        _onEnable.Connect(slot);
}

void RetroFuturaGUI::IWidget::Connect_OnDisable(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onDisableAsync.Connect(slot);
    else
        _onDisable.Connect(slot);
}

void RetroFuturaGUI::IWidget::Disconnect_OnEnable(const typename Signal<>::Slot& slot)
{
    _onEnable.Disconnect(slot);
    _onEnableAsync.Disconnect(slot);
}

void RetroFuturaGUI::IWidget::Disconnect_OnDisable(const typename Signal<>::Slot& slot)
{
    _onDisable.Disconnect(slot);
    _onDisableAsync.Disconnect(slot);
}

void RetroFuturaGUI::IWidget::SetSize(const glm::vec3& size)
{
    _size = size;
}

glm::vec3 RetroFuturaGUI::IWidget::GetSize() const
{
    return _size;
}

void RetroFuturaGUI::IWidget::SetPosition(const glm::vec3& position)
{
    _position = position;
}

glm::vec3 RetroFuturaGUI::IWidget::GetPosition() const
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
