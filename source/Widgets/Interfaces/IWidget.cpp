#include "IWidget.hpp"
#include "PlatformBridge.hpp"
#include "IDropDown.hpp"
#include "Tooltip.hpp"
#include "ContextMenu.hpp"
#include "FillStyle.hpp"
#include "Window.hpp"
#include "Shader.hpp"
#include <algorithm>
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
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Size) };

    //Before the first frame, without a transition, inside an InstantScope (a container laying the widget out) or with nothing to move, it applies at once, as a plain setter would
    if(!_hasUpdated || 0 >= transition._Duration.count() || (!_isResizing && size == _size))
    {
        _isResizing = false;
        _sizeTransition.Jump(size);
        const Transitions::InstantScope instant { true };
        applySize(size);
        return;
    }

    if(!_isResizing)
        _sizeTransition.Jump(_size); //from where it is now

    _sizeTransition.Set(size, transition);
    _isResizing = true;
}

glm::vec3 RetroFuturaGUI::IWidget::GetSize() const
{
    return _size;
}

void RetroFuturaGUI::IWidget::SetPosition(const glm::vec3& position)
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Position) };

    if(!_hasUpdated || 0 >= transition._Duration.count() || (!_isMoving && position == _position))
    {
        _isMoving = false;
        _positionTransition.Jump(position);
        const Transitions::InstantScope instant { true };
        applyPosition(position);
        return;
    }

    if(!_isMoving)
        _positionTransition.Jump(_position);

    _positionTransition.Set(position, transition);
    _isMoving = true;
}

glm::vec3 RetroFuturaGUI::IWidget::GetPosition() const
{
    return _position;
}

void RetroFuturaGUI::IWidget::SetRotation(const glm::vec3& rotation)
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Rotation) };

    if(!_hasUpdated || 0 >= transition._Duration.count() || (!_isTurning && rotation == _rotation))
    {
        _isTurning = false;
        _rotationTransition.Jump(rotation);
        const Transitions::InstantScope instant { true };
        applyRotation(rotation);
        return;
    }

    if(!_isTurning)
        _rotationTransition.Jump(_rotation);

    _rotationTransition.Set(rotation, transition);
    _isTurning = true;
}

void RetroFuturaGUI::IWidget::applySize(const glm::vec3& size)
{
    _size = size;
}

void RetroFuturaGUI::IWidget::applyPosition(const glm::vec3& position)
{
    _position = position;
}

void RetroFuturaGUI::IWidget::applyRotation(const glm::vec3& rotation)
{
    _rotation = rotation;
}

void RetroFuturaGUI::IWidget::updateGeometry()
{
    //Each value is read before the InstantScope, inside which it would read as arrived
    if(_isResizing)
    {
        const glm::vec3 size { _sizeTransition.Get() };
        _isResizing = _sizeTransition.IsRunning();
        const Transitions::InstantScope instant { true };
        applySize(size);
    }

    if(_isMoving)
    {
        const glm::vec3 position { _positionTransition.Get() };
        _isMoving = _positionTransition.IsRunning();
        const Transitions::InstantScope instant { true };
        applyPosition(position);
    }

    if(_isTurning)
    {
        const glm::vec3 rotation { _rotationTransition.Get() };
        _isTurning = _rotationTransition.IsRunning();
        const Transitions::InstantScope instant { true };
        applyRotation(rotation);
    }

    if(_isFading)
    {
        _opacity = _opacityTransition.Get();
        _isFading = _opacityTransition.IsRunning();
    }
}

void RetroFuturaGUI::IWidget::DrawAtOpacity()
{
    if(0.0f >= _opacity)
        return; //nothing of it would show

    const Shader::OpacityScope opacity { _opacity };
    const Shader::FrameScope frame { getTilt(), Shader::FrameComposition::Inside };
    _frame = Shader::GetDrawFrame();
    Draw();
}

glm::mat4 RetroFuturaGUI::IWidget::getTilt() const
{
    if(glm::vec3(0.0f) == _rotation)
        return glm::mat4(1.0f);

    // Turned around the center in the same Z * Y * X order every element composes its own rotation in
    const glm::vec3 radians { glm::radians(_rotation) };
    return glm::translate(glm::mat4(1.0f), _position)
        * glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f))
        * glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::translate(glm::mat4(1.0f), -_position);
}

std::optional<glm::vec2> RetroFuturaGUI::IWidget::toLayoutPoint(const glm::vec2& windowPoint, const glm::mat4& frame) const
{
    // The view through the window point from the near to the far end of the view-span. Taken back into the untilted layout
    const glm::vec2 resolution { glm::max(_projection.GetResolution(), glm::vec2(1.0f)) };
    const glm::vec2 clip { windowPoint / resolution * 2.0f - 1.0f };
    const glm::mat4 toLayout { glm::inverse(_projection.GetProjectionMatrix() * frame) };
    const glm::vec4
        nearPoint { toLayout * glm::vec4(clip, -1.0f, 1.0f) },
        farPoint { toLayout * glm::vec4(clip, 1.0f, 1.0f) };
    const glm::vec3
        rayStart { glm::vec3(nearPoint) / nearPoint.w },
        rayEnd { glm::vec3(farPoint) / farPoint.w };

    if(std::abs(rayEnd.z - rayStart.z) < 1e-6f)
        return std::nullopt; //the layout's plane is edge-on to the view

    // Where the ray meets the layout's plane at the widget's depth
    const f32 along { (_position.z - rayStart.z) / (rayEnd.z - rayStart.z) };
    return glm::vec2(rayStart + (rayEnd - rayStart) * along);
}

std::optional<glm::vec2> RetroFuturaGUI::IWidget::toOwnLayoutPoint(const glm::vec2& windowPoint) const
{
    return toLayoutPoint(windowPoint, _frame);
}

void RetroFuturaGUI::IWidget::SetOpacity(const f32 opacity)
{
    const f32 target { std::clamp(opacity, 0.0f, 1.0f) };
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Opacity) };

    if(!_hasUpdated || 0 >= transition._Duration.count())
    {
        _isFading = false;
        _opacityTransition.Jump(target);
        _opacity = target;
        return;
    }

    if(!_isFading)
        _opacityTransition.Jump(_opacity); //from where it is now

    _opacityTransition.Set(target, transition);
    _isFading = true;
}

f32 RetroFuturaGUI::IWidget::GetOpacity() const
{
    return _opacity;
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

GLFWwindow* RetroFuturaGUI::IWidget::GetParentWindow() const
{
    return _parentWindow;
}

void RetroFuturaGUI::IWidget::SetEnabled(const bool enable, const bool emitSignal)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        if(emitSignal)
        {
            _onEnableAsync.EmitAsync();
            _onEnable.Emit();
        }

        setColors(ColorState::Enabled);
        return;
    }

    if(emitSignal)
    {
        _onDisableAsync.EmitAsync();
        _onDisable.Emit();
    }

    setColors(ColorState::Disabled);
}

bool RetroFuturaGUI::IWidget::IsEnabled() const
{
    return _isEnabledFlag;
}

bool RetroFuturaGUI::IWidget::isPointInsideRect(const glm::vec2& point, const glm::vec3& size, const glm::vec3& position, const glm::vec3& rotation) const
{
    // The point is in the widget's untilted layout (readPointer), so only a part's own turn within it applies here
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
    return isPointInsideRect(point, _size, _position, glm::vec3(0.0f)); //the widget's own rotation is in its frame
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

    // PlatformBridge reports native, top-down window coordinates; the library's world space runs bottom-up. The widget sees
    // the point where the pointer meets its untilted layout. For an untilted widget in an orthographic view, it's the same point
    pointer._WindowPosition = glm::vec2(static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY));
    const std::optional<glm::vec2> layoutPoint { toOwnLayoutPoint(pointer._WindowPosition) };
    pointer._HasPosition = pointer._HasPosition && layoutPoint.has_value();
    pointer._Position = layoutPoint.value_or(pointer._WindowPosition);
    pointer._LeftButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left);
    pointer._RightButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Right);
    Window::ReadWheel(_parentWindow, pointer);

    filterThroughAncestors(_parentWidget, this, pointer);
    IDropDown::CaptureUnderOpenList(this, _parentWindow, pointer);
    return pointer;
}

void RetroFuturaGUI::IWidget::requestCursorIcon(const PlatformBridge::CursorIcon icon) const
{
    Window::RequestCursorIcon(_parentWindow, icon);
}

void RetroFuturaGUI::IWidget::filterThroughAncestors(const IHierarchyNode* node, const IHierarchyNode* reader, PointerState& pointer)
{
    if(!node)
        return;

    filterThroughAncestors(node->GetParent(), reader, pointer);

    const IPointerFilter* filter { dynamic_cast<const IPointerFilter*>(node) };

    if(!filter)
        return;

    filter->FilterPointer(pointer, reader);
}

void RetroFuturaGUI::IWidget::SetTooltipText(std::string_view text)
{
    if(!_tooltip)
        _tooltip = std::make_unique<Tooltip>(&_projection, this);

    if(!_tooltip)
        return;

    _tooltip->SetText(text, false);
}

void RetroFuturaGUI::IWidget::ShowTooltip(const bool show)
{
    _enableTooltip = show;
}

RetroFuturaGUI::Tooltip* RetroFuturaGUI::IWidget::GetTooltip() const
{
    return _tooltip.get();
}

void RetroFuturaGUI::IWidget::Interact()
{
    const Shader::FrameScope frame { getTilt(), Shader::FrameComposition::Inside };
    _frame = Shader::GetDrawFrame();
    updateTooltip();
    interact();
}

void RetroFuturaGUI::IWidget::Update()
{
    _hasUpdated = true;
    updateGeometry();
    update();
}

void RetroFuturaGUI::IWidget::updateTooltip()
{
    if(_enableTooltip && _tooltip)
        _tooltip->Update();

    updateContextMenu();
}

void RetroFuturaGUI::IWidget::AddContextMenuItem(std::string_view itemName, std::string_view node, std::string_view imagePath, std::string_view text, std::string_view note)
{
    if(!_contextMenu)
        _contextMenu = std::make_unique<ContextMenu>(&_projection, this);

    if(!_contextMenu)
        return;

    _contextMenu->AddItem(itemName, node, imagePath, text, note);
}

void RetroFuturaGUI::IWidget::AddContextMenuNode(std::string_view newNodeName, std::string_view node)
{
    if(!_contextMenu)
        _contextMenu = std::make_unique<ContextMenu>(&_projection, this);

    if(!_contextMenu)
        return;

    _contextMenu->AddNode(newNodeName, node);
}

void RetroFuturaGUI::IWidget::SetTransition(const TransitionProperty properties, const Transition& transition)
{
    Transitions::Override(_transitionSettings, properties, transition);
}

void RetroFuturaGUI::IWidget::ResetTransition(const TransitionProperty properties)
{
    Transitions::Reset(_transitionSettings, properties);
}

RetroFuturaGUI::Transition RetroFuturaGUI::IWidget::GetTransition(const TransitionProperty property) const
{
    return Transitions::Resolve(_transitionSettings, property);
}

void RetroFuturaGUI::IWidget::useTransitions(std::initializer_list<FillStyle*> elements)
{
    for(FillStyle* element : elements)
    {
        if(!element)
            continue;

        element->SetTransitionSettings(&_transitionSettings);
    }
}

RetroFuturaGUI::ContextMenu* RetroFuturaGUI::IWidget::GetContextMenu() const
{
    return _contextMenu.get();
}

void RetroFuturaGUI::IWidget::updateContextMenu()
{
    if(!_contextMenu)
        return;

    _contextMenu->Update();
}