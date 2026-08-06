#include "IWidget.hpp"

RetroFuturaGUI::IWidget::IWidget(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : _name(name), _parentWidget(parentWidget), _parentWidgetTypeID(parentWidgetTypeID), _projection(*projection)
{
    _parentWindow = parentWindow;
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

void RetroFuturaGUI::IWidget::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;
}

glm::vec3 RetroFuturaGUI::IWidget::GetRotation() const
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

bool RetroFuturaGUI::IWidget::isPointInside(const glm::vec2& point) const
{
    /* Hit-testing is a 2D point-in-rectangle check, so only the Z-axis (screen-plane)
       rotation applies here - X/Y rotation tilts the widget in a way this simple test
       can't represent*/
    glm::vec2 translatedPoint = point - glm::vec2(_position.x, _position.y);
    f32 radians = glm::radians(-_rotation.z);
    glm::vec2 rotatedPoint(
        translatedPoint.x * cos(radians) - translatedPoint.y * sin(radians),
        translatedPoint.x * sin(radians) + translatedPoint.y * cos(radians)
    );
    return (rotatedPoint.x >= -_size.x * 0.5f &&
            rotatedPoint.x <= _size.x * 0.5f &&
            rotatedPoint.y >= -_size.y * 0.5f &&
            rotatedPoint.y <= _size.y * 0.5f);
}