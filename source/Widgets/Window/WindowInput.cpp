#include "Window.hpp"
#include "WindowTab.hpp"
#include "Scene.hpp"
#include "PlatformBridge.hpp"
#include <algorithm>
#include <utility>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

void RetroFuturaGUI::Window::RequestCursorIcon(GLFWwindow* glfwWindow, const PlatformBridge::CursorIcon icon)
{
    //The window keys arrive at is the one on screen: the host's while this one sits in a WindowTab
    Window* window { FromGlfwWindow(KeyboardWindowOf(glfwWindow)) };

    if(!window)
        return;

    window->_requestedCursorIcon = icon;
}

void RetroFuturaGUI::Window::ReadWheel(GLFWwindow* glfwWindow, PointerState& pointer)
{
    pointer._Wheel = glm::vec2(0.0f);
    pointer._PinchScale = 1.0f;

    //The window on screen, which is the host's while this one sits in a tab. It reads the mouse each frame, so it keeps what was taken
    const Window* window { FromGlfwWindow(KeyboardWindowOf(glfwWindow)) };

    if(!window)
        return;

    if(!window->_hasPointer)
        return;

    const PlatformBridge::WheelDelta wheel { PlatformBridge::Input::GetWheelDelta() };

    if(!HasWheelPart(window->_takenWheelParts, WheelParts::Horizontal))
        pointer._Wheel.x = wheel._X;

    if(!HasWheelPart(window->_takenWheelParts, WheelParts::Vertical))
        pointer._Wheel.y = wheel._Y;

    if(!HasWheelPart(window->_takenWheelParts, WheelParts::Pinch))
        pointer._PinchScale = PlatformBridge::Input::GetPinchScale();
}

void RetroFuturaGUI::Window::TakeWheel(GLFWwindow* glfwWindow, const WheelParts parts)
{
    Window* window { FromGlfwWindow(KeyboardWindowOf(glfwWindow)) };

    if(!window)
        return;

    window->_takenWheelParts = window->_takenWheelParts | parts;
}

GLFWwindow* RetroFuturaGUI::Window::GetKeyboardWindow() const
{
    const Window* host { getTabHostWindow() };

    if(!host)
        return _window;

    return host->GetKeyboardWindow();
}

void RetroFuturaGUI::Window::TakeKeyboardInput() const
{
    GLFWwindow* keyboardWindow { GetKeyboardWindow() };

    if(!keyboardWindow)
        return;

#if defined(TARGET_PLATFORM_LINUX)
    PlatformBridge::Input::SetActiveDisplay(glfwGetX11Display());
    PlatformBridge::Input::SetActiveWindow(glfwGetX11Window(keyboardWindow));
#elif defined(TARGET_PLATFORM_WINDOWS)
    PlatformBridge::Input::SetActiveWindow(glfwGetWin32Window(keyboardWindow));
#endif
}

GLFWwindow* RetroFuturaGUI::Window::KeyboardWindowOf(GLFWwindow* glfwWindow)
{
    const Window* window { FromGlfwWindow(glfwWindow) };

    if(!window)
        return glfwWindow;

    return window->GetKeyboardWindow();
}

void RetroFuturaGUI::Window::FilterPointer(PointerState& pointer) const
{
    //The mouse is on another window lying over this one, or holds a press that began on another window: to this window's
    //widgets it is outside, so they let go of their hover. The press stays visible, so a text box still ends its editing.
    if(!_hasPointer)
        pointer._HasPosition = false;

    if(_ignoreHeldPress)
        pointer._LeftButtonPressed = false;

    //Only the capture, so a press stays visible and a text box below still ends its editing on a click elsewhere
    if(IsBlockedByPopup())
        pointer._IsCaptured = true;

    if(!_tabHost)
        return;

    if(!_tabHost->IsEnabled())
        pointer._IsCaptured = true;
}

void RetroFuturaGUI::Window::Connect_OnFilesDropped(const typename Signal<std::vector<std::string>, glm::vec2>::Slot& slot, const bool async)
{
    if(async)
        _onFilesDroppedAsync.Connect(slot);
    else
        _onFilesDropped.Connect(slot);
}

void RetroFuturaGUI::Window::Disconnect_OnFilesDropped(const typename Signal<std::vector<std::string>, glm::vec2>::Slot& slot)
{
    _onFilesDropped.Disconnect(slot);
    _onFilesDroppedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Window::pollInput()
{
    const bool isMouseButtonDown { PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left) };
    _takenWheelParts = WheelParts::None; //a new frame's wheel

    //The mouse is read for the whole screen, so only the topmost window under the cursor takes it, and a press stays with the
    //window it landed on until it is let go: a click never reaches a window lying underneath, and a drag carries on over others
    if(!isMouseButtonDown || !_wasMouseButtonDown)
        _hasPointer = isUnderMouse();

    setCursorPosition();

    //Once the button is up, a press held from before this window took the mouse is over
    if(!isMouseButtonDown)
        _ignoreHeldPress = false;

    if (isMouseButtonDown && !_wasMouseButtonDown && !_ignoreHeldPress && _hasPointer) //press
        setResizeState(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
    else if (!isMouseButtonDown && _wasMouseButtonDown) //release
        setResizeState(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);

    _wasMouseButtonDown = isMouseButtonDown;

    resize();
}

bool RetroFuturaGUI::Window::isUnderMouse() const
{
    if(!_window)
        return false;

#if defined(TARGET_PLATFORM_LINUX)
    return PlatformBridge::Input::IsWindowUnderMouse(glfwGetX11Window(_window));
#elif defined(TARGET_PLATFORM_WINDOWS)
    return PlatformBridge::Input::IsWindowUnderMouse(glfwGetWin32Window(_window));
#else
    return true;
#endif
}

bool RetroFuturaGUI::Window::getCursorPosition(i32& x, i32& y) const
{
#if defined(TARGET_PLATFORM_LINUX)
    return PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_window), x, y);
#elif defined(TARGET_PLATFORM_WINDOWS)
    return PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_window), x, y);
#else
    return false;
#endif
}

void RetroFuturaGUI::Window::setCursorPosition()
{
    i32 x { 0 }, y { 0 };
    getCursorPosition(x, y);
    _cursorPosX = x;
    _cursorPosY = y;
}

void RetroFuturaGUI::Window::setCursorIcon()
{
    if(!_window)
        return;

    ResizeEdge edge { resizeEdgeAt(_cursorPosX, _cursorPosY) };

    if(!canResize(edge))
        edge = ResizeEdge::None;

    PlatformBridge::CursorIcon icon { _requestedCursorIcon }; //the widgets' icon, unless the mouse is at an edge the window resizes from
    _requestedCursorIcon = PlatformBridge::CursorIcon::Arrow; //each frame's widgets ask anew

    switch(edge)
    {
        case ResizeEdge::TopLeft:
        case ResizeEdge::BottomRight:
            icon = PlatformBridge::CursorIcon::ResizeTopLeftBottomRight;
        break;
        case ResizeEdge::TopRight:
        case ResizeEdge::BottomLeft:
            icon = PlatformBridge::CursorIcon::ResizeTopRightBottomLeft;
        break;
        case ResizeEdge::Left:
        case ResizeEdge::Right:
            icon = PlatformBridge::CursorIcon::ResizeHorizontal;
        break;
        case ResizeEdge::Top:
        case ResizeEdge::Bottom:
            icon = PlatformBridge::CursorIcon::ResizeVertical;
        break;
        default:
        break;
    }

    if(icon == _cursorIcon)
        return;

    _cursorIcon = icon;

#if defined(TARGET_PLATFORM_LINUX)
    PlatformBridge::Cursor::SetIcon(glfwGetX11Display(), glfwGetX11Window(_window), icon);
#elif defined(TARGET_PLATFORM_WINDOWS)
    PlatformBridge::Cursor::SetIcon(glfwGetWin32Window(_window), icon);
#endif
}

void RetroFuturaGUI::Window::setResizeState(i32 button, i32 action, [[maybe_unused]] i32 mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        i32 x { 0 }, y { 0 };
        getCursorPosition(x, y);
        glfwGetWindowPos(_window, &_windowPosX, &_windowPosY);
        _absoluteCursorPosX = x + _windowPosX;
        _absoluteCursorPosY = y + _windowPosY;
        _cursorPosX = x;
        _cursorPosY = y;
        _prevResizeX = _absoluteCursorPosX;
        _prevResizeY = _absoluteCursorPosY;

        //The button is read for the whole screen, so a press in another window arrives here too. Outside this window every
        //position would count as near an edge, so only a press inside it may start a resize.
        if(0.0 > _cursorPosX || 0.0 > _cursorPosY || static_cast<f64>(_width) <= _cursorPosX || static_cast<f64>(_height) <= _cursorPosY)
            return;

        _resizeEdge = resizeEdgeAt(_cursorPosX, _cursorPosY);

        if(!canResize(_resizeEdge))
            _resizeEdge = ResizeEdge::None;

        if (_resizeEdge != ResizeEdge::None)
            _isResizing = true;
    } 
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        _isResizing = false;
        _resizeEdge = ResizeEdge::None;
    }
}

void RetroFuturaGUI::Window::resize()
{
    if (!_isResizing) 
        return;

    i32 rx { 0 }, ry { 0 };
    getCursorPosition(rx, ry);
    glfwGetWindowPos(_window, &_windowPosX, &_windowPosY);
    _absoluteCursorPosX = rx + _windowPosX;
    _absoluteCursorPosY = ry + _windowPosY;

    f64
        deltaX { _absoluteCursorPosX - _prevResizeX },
        deltaY { _absoluteCursorPosY - _prevResizeY };
    i32
        newWidth { _width },
        newHeight { _height },
        newPosX { _windowPosX },
        newPosY { _windowPosY };

    switch(_resizeEdge)
    {
        case ResizeEdge::TopLeft:
            newWidth = _width - (i32)deltaX;
            newHeight = _height - (i32)deltaY;
            newPosX = _windowPosX + (i32)deltaX;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::TopRight:
            newWidth = _width + (i32)deltaX;
            newHeight = _height - (i32)deltaY;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::BottomLeft:
            newWidth = _width - (i32)deltaX;
            newHeight = _height + (i32)deltaY;
            newPosX = _windowPosX + (i32)deltaX;
        break;
        case ResizeEdge::BottomRight:
            newWidth = _width + (i32)deltaX;
            newHeight = _height + (i32)deltaY;
        break;
        case ResizeEdge::Left:
            newWidth = _width - (i32)deltaX;
            newPosX = _windowPosX + (i32)deltaX;
        break;
        case ResizeEdge::Right:
            newWidth = _width + (i32)deltaX;
        break;
        case ResizeEdge::Top:
            newHeight = _height - (i32)deltaY;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::Bottom:
            newHeight = _height + (i32)deltaY;
        break;
        default:
        break;
    }

    _prevResizeX = _absoluteCursorPosX;
    _prevResizeY = _absoluteCursorPosY;
    _width = std::max<i32>(newWidth, _minWindowSpan);
    _height = std::max<i32>(newHeight, _minWindowSpan);
    _windowPosX = newPosX;
    _windowPosY = newPosY;
    glfwSetWindowPos(_window, _windowPosX, _windowPosY);
    glfwSetWindowSize(_window, _width, _height);
    _windowSizeChanged = true;
}

RetroFuturaGUI::Window::ResizeEdge RetroFuturaGUI::Window::resizeEdgeAt(const f64 x, const f64 y) const
{
    const bool
        nearLeft { x < _boundaryThreshold },
        nearRight { x > _width - _boundaryThreshold },
        nearTop { y < _boundaryThreshold },
        nearBottom { y > _height - _boundaryThreshold };

    //Corners first, so the cursor near one resizes both of its edges
    if(nearTop && nearLeft)
        return ResizeEdge::TopLeft;

    if(nearTop && nearRight)
        return ResizeEdge::TopRight;

    if(nearBottom && nearLeft)
        return ResizeEdge::BottomLeft;

    if(nearBottom && nearRight)
        return ResizeEdge::BottomRight;

    if(nearLeft)
        return ResizeEdge::Left;

    if(nearRight)
        return ResizeEdge::Right;

    if(nearTop)
        return ResizeEdge::Top;

    if(nearBottom)
        return ResizeEdge::Bottom;

    return ResizeEdge::None;
}

bool RetroFuturaGUI::Window::canResize(const ResizeEdge edge) const
{
    if(IsInTab())
        return false;

    if(IsBlockedByPopup())
        return false;

    if(!_dockHost)
        return true;

    switch(_dockHostEdge)
    {
        case DockEdge::Top:
            return ResizeEdge::Top == edge;
        case DockEdge::Right:
            return ResizeEdge::Right == edge;
        case DockEdge::Bottom:
            return ResizeEdge::Bottom == edge;
        case DockEdge::Left:
            return ResizeEdge::Left == edge;
        default:
            return false;
    }
}

void RetroFuturaGUI::Window::dropCallback(GLFWwindow* glfwWindow, const int pathCount, const char* paths[])
{
    Window* window { FromGlfwWindow(glfwWindow) };

    if(!window)
        return;

    DroppedFiles dropped {};

    for(int index = 0; index < pathCount; ++index)
    {
        if(!paths[index])
            continue;

        dropped._Paths.emplace_back(paths[index]);
    }

    // pointer where the files were released
    i32
        x { 0 },
        y { 0 };

    if(window->getCursorPosition(x, y) && window->_projection)
        dropped._Position = glm::vec2(static_cast<f32>(x), window->_projection->GetResolution().y - static_cast<f32>(y));

    window->_droppedFiles.push_back(std::move(dropped));
}

void RetroFuturaGUI::Window::emitDroppedFiles()
{
    if(_droppedFiles.empty())
        return;

    //Taken off first, since a slot may open a popup or drop again
    std::vector<DroppedFiles> dropped { std::move(_droppedFiles) };
    _droppedFiles.clear();

    if(IsBlockedByPopup())
        return;

    for(const DroppedFiles& files : dropped)
    {
        _onFilesDroppedAsync.EmitAsync(files._Paths, files._Position);
        _onFilesDropped.Emit(files._Paths, files._Position);
    }
}

void RetroFuturaGUI::Window::interactContent()
{
    interactLasagna();

    for(Scene* scene : _scenes)
    {
        if(!isSceneShown(scene))
            continue;

        scene->Interact();
    }
}

void RetroFuturaGUI::Window::interactLasagna()
{
    if(!_lasagna)
        return;

    _lasagna->Interact();
}