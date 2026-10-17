#include "IWidget.hpp"
#include "PlatformBridge.hpp"
#include "Tooltip.hpp"
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::IWidget::IWidget(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : _name(name), _parentWidget(parentWidget), _parentWidgetTypeID(parentWidgetTypeID), _projection(*projection)
{
    _parentWindow = parentWindow;
}

RetroFuturaGUI::IWidget::~IWidget() = default;

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

const std::string& RetroFuturaGUI::IWidget::GetName() const
{
    return _name;
}

const RetroFuturaGUI::IHierarchyNode* RetroFuturaGUI::IWidget::GetParent() const
{
    return _parentWidget;
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

bool RetroFuturaGUI::IWidget::isPointInsideRect(const glm::vec2& point, const glm::vec3& size, const glm::vec3& position, const glm::vec3& rotation) const
{
     /* Hit-testing is a 2D point-in-rectangle check, so only the Z-axis (screen-plane)
       rotation applies here - X/Y rotation tilts the widget in a way this simple test
       can't represent*/
    glm::vec2 translatedPoint = point - glm::vec2(position.x, position.y);
    f32 radians = glm::radians(- rotation.z);
    glm::vec2 rotatedPoint(
        translatedPoint.x * cos(radians) - translatedPoint.y * sin(radians),
        translatedPoint.x * sin(radians) + translatedPoint.y * cos(radians)
    );
    return (rotatedPoint.x >= -size.x * 0.5f &&
            rotatedPoint.x <= size.x * 0.5f &&
            rotatedPoint.y >= -size.y * 0.5f &&
            rotatedPoint.y <= size.y * 0.5f);
}

bool RetroFuturaGUI::IWidget::isPointInside(const glm::vec2& point) const
{
    return isPointInsideRect(point, _size, _position, _rotation);
}

RetroFuturaGUI::PointerState RetroFuturaGUI::IWidget::readPointer() const
{
    PointerState pointer {};
    i32
        mouseX { 0 },
        mouseY { 0 };

#if defined(TARGET_PLATFORM_LINUX)
    pointer._HasPosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_parentWindow), mouseX, mouseY);
#elif defined(TARGET_PLATFORM_WINDOWS)
    pointer._HasPosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_parentWindow), mouseX, mouseY);
#endif

    // PlatformBridge reports native, top-down window coordinates; the library's world space runs bottom-up.
    pointer._Position = glm::vec2(static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY));
    pointer._LeftButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left);

    filterThroughAncestors(_parentWidget, pointer);
    return pointer;
}

void RetroFuturaGUI::IWidget::filterThroughAncestors(const IHierarchyNode* node, PointerState& pointer)
{
    if(!node)
        return;

    filterThroughAncestors(node->GetParent(), pointer);

    const IPointerFilter* filter { dynamic_cast<const IPointerFilter*>(node) };

    if(!filter)
        return;

    filter->FilterPointer(pointer);
}

void RetroFuturaGUI::IWidget::SetTooltipText(std::string_view text)
{
    if(!_tooltip)
        _tooltip = std::make_unique<Tooltip>(&_projection, this);

    if(!_tooltip)
        return;

    _tooltip->SetText(text, false);
}

void RetroFuturaGUI::IWidget::EnableTooltip(const bool enable)
{
    _enableTooltip = enable;
}

RetroFuturaGUI::Tooltip* RetroFuturaGUI::IWidget::GetTooltip() const
{
    return _tooltip.get();
}

void RetroFuturaGUI::IWidget::UpdateTooltip()
{
    if(!_enableTooltip)
        return;

    if(!_tooltip)
        return;

    _tooltip->Update();
}