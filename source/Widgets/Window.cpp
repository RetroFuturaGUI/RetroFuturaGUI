#include "Window.hpp"
#include "Scene.hpp"
#include "WindowTab.hpp"
#include "ContextObjects.hpp"
#include "ShaderManager.hpp"
#include "PlatformBridge.hpp"
#include "IDropDown.hpp"
#include "Tooltip.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <print>
#include <utility>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::Window::Window(std::string_view name, const i32 width, const i32 height)
: Window(name, width, height, WindowRole::Normal)
{
}

RetroFuturaGUI::Window::Window(std::string_view name, const i32 width, const i32 height, const WindowRole role)
: _width(width), _height(height), _name(name)
{
    _role = role;
    createWindow();

    //Docking previews and popups are never docking targets
    if(WindowRole::Normal == _role)
        allWindows().push_back(this);
}

RetroFuturaGUI::Window::~Window()
{
    //Popups go from the top down, the reverse of how they opened, which a vector's own destruction wouldn't do. Their results
    //don't go out: the slots may be gone by now.
    while(!_popups.empty())
        _popups.pop_back();

    if(_dockSnap._Tab && isRegisteredWindowTab(_dockSnap._Tab))
        _dockSnap._Tab->hideDropPreview();

    std::erase(allWindows(), this);
    Undock();

    if(!_window)
        return;

    //The background's and the border's GL objects belong to this window's context, so they go while it still exists and is current
    glfwMakeContextCurrent(_window);
    ContextObject::DeletePending(_window);
    _background.reset();
    _backgroundImage.reset();
    _border.reset();

    //A later window must not try to share with a destroyed one
    if(_shareWindow == _window)
        _shareWindow = nullptr;

    glfwDestroyWindow(_window);
    ContextObject::ForgetContext(_window); //its VAOs and framebuffers went with it
}

void RetroFuturaGUI::Window::createWindow()
{
    //Hints outlive the window they were set for, so every one this window depends on is set each time
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    //enable transparency on cut out corners
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    //A docking preview shows only once it is placed, stays above the windows it previews and never takes the focus. A popup
    //shows once it is centered and has drawn, and takes the focus then.
    glfwWindowHint(GLFW_VISIBLE, WindowRole::Normal == _role ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, WindowRole::DockPreview == _role ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, WindowRole::DockPreview == _role ? GLFW_FALSE : GLFW_TRUE);

    //Shares GL objects with the first window, so the shaders compiled once below and the fonts and images loaded there work in this one too.
    //VAOs are never shared, so a widget still has to be built, drawn and destroyed with its own window's context current.
    _window = glfwCreateWindow(_width, _height, _windowTitle.c_str(), nullptr, _shareWindow);
    glfwSetWindowTitle(_window, _windowTitle.c_str());

    if (!_window)
    {
        std::println("{0}", "Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    if(!_shareWindow)
        _shareWindow = _window;

    //Lets a window bar, which only gets the GLFW window, reach this one, e.g. to undock it
    glfwSetWindowUserPointer(_window, this);
    glfwSetDropCallback(_window, &Window::dropCallback);

    _prevResizeX = (f64)_width;
    _prevResizeY = (f64)_height;

    glfwMakeContextCurrent(_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::println("Failed to initialize GLAD in Window::createWindow");
        glfwDestroyWindow(_window);
        _window = nullptr;
        glfwTerminate();
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //what is left where nothing draws, the corners outside the background: invisible
    glViewport(0, 0, _width, _height);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::println("Failed to initialize GLAD");
        return;
    }

    static bool shadersInitialized = false;
    if (!shadersInitialized)
    {
        ShaderManager::Init();
        shadersInitialized = true;
    }

    _projection = std::make_unique<Projection>((float)_width, (float)_height);

    _background = std::make_unique<Rectangle>(_projection.get());

    if(_background)
    {
        _background->SetRectangleMode(RectangleMode::Plane);
        _background->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -1.0f));
        _background->SetSize(glm::vec2((f32)_width, (f32)_height));
        _background->SetRotation(glm::vec3(0.0f));
    }

    _border = std::make_unique<Rectangle>(_projection.get());

    if(_border)
    {
        _border->SetRectangleMode(RectangleMode::Border);
        _border->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, 0.0f));
        _border->SetSize(glm::vec2((f32)_width, (f32)_height));
        _border->SetRotation(glm::vec3(0.0f));
        //No frame until one is asked for, since a Rectangle starts with a 5 px border
        _border->SetBorderWidth(0.0f);
    }

    SetBackgroundColor(glm::vec4(0.086f, 0.086f, 0.1137f, 1.0f), ColorState::Enabled);
    setBorderColors();
}

void RetroFuturaGUI::Window::SetWindowTitle(std::string_view title)
{
    if(!_window)
        return;

    _windowTitle = title;
    glfwSetWindowTitle(_window, _windowTitle.c_str());
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

void RetroFuturaGUI::Window::pollInput()
{
    const bool isMouseButtonDown { PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left) };

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

void RetroFuturaGUI::Window::updateProjection()
{
    if(!updateSize())
        return;

    glViewport(0, 0, _width, _height);
}

bool RetroFuturaGUI::Window::updateSize()
{
    if(!_window)
        return false;

    if(!_projection)
        return false;

    glfwGetWindowSize(_window, &_width, &_height);

    if (_lastSize == glm::ivec2(_width, _height))
        return false;

    _projection->UpdateProjectionMatrix((f32)_width, (f32)_height);
    UpdateLayout();

    if(_background)
    {
        _background->SetSize(glm::vec2((f32)_width, (f32)_height));
        _background->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -1.0f));
    }

    if(_border)
    {
        _border->SetSize(glm::vec2((f32)_width, (f32)_height));
        _border->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, 0.0f));
    }

    _lastSize = { _width, _height };
    _windowSizeChanged = false;
    return true;
}

bool RetroFuturaGUI::Window::WindowShouldClose()
{
    if(!_window)
        return true;

    refuseCloseWhileBlocked();
    return GLFW_TRUE == glfwWindowShouldClose(_window);
}

void RetroFuturaGUI::Window::Draw()
{
    if(!_window)
        return;

    if(IsInTab())
        return; //the window holding its WindowTab draws it

    //Another window may have drawn last, and every GL call below has to land in this window's context
    glfwMakeContextCurrent(_window);
    ContextObject::DeletePending(_window); //VAOs and framebuffers of objects destroyed while another window's context was current

    //Each WindowTab marks itself again when it draws. One in an inactive scene, which doesn't, takes no dropped windows.
    for(WindowTab* tab : _windowTabs)
    {
        if(!tab)
            continue;

        tab->_isShown = false;
    }

    if(WindowRole::DockPreview != _role)
        pollInput();

    emitDroppedFiles(); //before the scenes, so a slot's changes show in this frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Color blends as usual. Alpha only ever adds up, so a semi-transparent widget on the opaque background leaves the window
    //opaque there, instead of lowering its alpha and letting the desktop show through. The result is premultiplied by alpha,
    //which is what the compositor expects of a transparent framebuffer.
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    updateProjection();
    drawContent();

    if(WindowRole::DockPreview != _role)
        setCursorIcon(); //after the content, whose widgets ask for their icons while they draw

    glfwSwapBuffers(_window);

    if(_showAfterDraw)
    {
        _showAfterDraw = false;
        glfwShowWindow(_window);
    }

    glfwPollEvents();
    refuseCloseWhileBlocked();
    followCursor(); //before the docking, which looks at where the window is now
    followDocking(); //After the scenes, because a window bar may have dragged this window while it drew
    updateDockSnap();
    drawPopups();
}

void RetroFuturaGUI::Window::drawContent()
{
    applyCornerRadii();
    drawBackground();

    if(_lasagna)
        _lasagna->Draw();

    for(Scene* scene : _scenes)
    {
        if(!isSceneShown(scene))
            continue;

        //Each scene starts with a fresh depth buffer, so a scene added later always draws over the ones loaded before
        glClear(GL_DEPTH_BUFFER_BIT);
        scene->Draw();
    }

    IDropDown::DrawShown(); //after every scene, so an open list covers the widgets drawn after its box
    drawFrame();
    Tooltip::DrawShown(); //last, so it covers every scene
}

void RetroFuturaGUI::Window::drawFrame()
{
    if(!_border)
        return;

    if(IsInTab())
        return; //it would cut the selected field off from the content

    //Width 0, the default, means no frame
    if(0.0f >= _border->GetBorderWidth())
        return;

    //A fresh depth buffer, so the frame lies over every scene
    glClear(GL_DEPTH_BUFFER_BIT);
    drawBorder();
}

void RetroFuturaGUI::Window::SetWindowSize(i32 width, i32 height)
{
    SetWidth(width);
    SetHeight(height);
}

void RetroFuturaGUI::Window::SetWidth(i32 width)
{
    if(IsInTab())
    {
        _untabbedSize.x = width;
        return;
    }

    _width = width;
    glfwSetWindowSize(_window, _width, _height);
}

void RetroFuturaGUI::Window::SetHeight(i32 height)
{
    if(IsInTab())
    {
        _untabbedSize.y = height;
        return;
    }

    _height = height;
    glfwSetWindowSize(_window, _width, _height);
}

void RetroFuturaGUI::Window::SetBackgroundImage(std::string_view imagePath)
{
    IBackground::SetBackgroundImage(imagePath);
}

void RetroFuturaGUI::Window::SetLasagna(Lasagna* lasagna)
{
    _lasagna = lasagna;
    UpdateLayout();
}

void RetroFuturaGUI::Window::AddScene(Scene* scene)
{
    if(!scene)
        return;

    if(std::find(_scenes.begin(), _scenes.end(), scene) != _scenes.end())
        return;

    _scenes.push_back(scene);
    UpdateLayout();
}

void RetroFuturaGUI::Window::RemoveScene(Scene* scene)
{
    if(!scene)
        return;

    const std::vector<Scene*>::iterator found { std::find(_scenes.begin(), _scenes.end(), scene) };

    if(found == _scenes.end())
        return;

    _scenes.erase(found);
    UpdateLayout();
}

const RetroFuturaGUI::ScreenRect& RetroFuturaGUI::Window::GetClientRect() const
{
    return _clientRect;
}

RetroFuturaGUI::ScreenRect RetroFuturaGUI::Window::reserveEdge(const DockEdge edge, const f32 thickness, ScreenRect& client)
{
    const bool horizontal { edge == DockEdge::Left || edge == DockEdge::Right };
    const f32 taken { std::clamp(thickness, 0.0f, horizontal ? client._Extent.x : client._Extent.y) };

    ScreenRect strip { ._Origin = client._Origin, ._Extent = client._Extent };

    switch(edge)
    {
    case DockEdge::Top:
        strip._Origin.y = client._Origin.y + client._Extent.y - taken;
        strip._Extent.y = taken;
        client._Extent.y -= taken;
    break;
    case DockEdge::Bottom:
        strip._Extent.y = taken;
        client._Origin.y += taken;
        client._Extent.y -= taken;
    break;
    case DockEdge::Left:
        strip._Extent.x = taken;
        client._Origin.x += taken;
        client._Extent.x -= taken;
    break;
    case DockEdge::Right:
        strip._Origin.x = client._Origin.x + client._Extent.x - taken;
        strip._Extent.x = taken;
        client._Extent.x -= taken;
    break;
    }

    return strip;
}

void RetroFuturaGUI::Window::UpdateLayout()
{
    ScreenRect client { ._Origin = glm::vec2(0.0f), ._Extent = glm::vec2((f32)_width, (f32)_height) };

    //Docked scenes take their strip in the order they were added, so the first one to claim an edge also owns the corner where two claims meet.
    //A scene hidden in a tab reserves nothing while the window is in one.
    for(Scene* scene : _scenes)
    {
        if(!isSceneShown(scene) || !scene->HasReservedEdge())
            continue;

        const ScreenRect strip { reserveEdge(scene->GetReservedEdge(), scene->GetReservedThickness(), client) };

        scene->SetPosition(glm::vec3(strip.Center(), scene->GetPosition().z));
        scene->SetSize(glm::vec3(strip._Extent, scene->GetSize().z));
    }

    _clientRect = client;

    //Overlay scenes and the root Lasagna share whatever the docked scenes left.
    for(Scene* scene : _scenes)
    {
        if(!scene || scene->HasReservedEdge())
            continue;

        scene->SetPosition(glm::vec3(client.Center(), scene->GetPosition().z));
        scene->SetSize(glm::vec3(client._Extent, scene->GetSize().z));
    }

    if(!_lasagna)
        return;

    _lasagna->SetPosition(glm::vec3(client.Center(), _lasagna->GetPosition().z));
    _lasagna->SetSize(glm::vec3(client._Extent, _lasagna->GetSize().z));
}

i32 RetroFuturaGUI::Window::GetBackgroundImageId() const
{
    if(_backgroundImage)
        return _backgroundImage->GetID();

    return -1;
}

GLFWwindow* RetroFuturaGUI::Window::GetGlfwWindow() const
{
    return _window;
}

RetroFuturaGUI::Projection* RetroFuturaGUI::Window::GetProjection() const
{
    return &*_projection;
}

const std::string& RetroFuturaGUI::Window::GetName() const
{
    return _name;
}

const RetroFuturaGUI::IHierarchyNode* RetroFuturaGUI::Window::GetParent() const
{
    return nullptr;
}

void RetroFuturaGUI::Window::ToggleMaximize()
{
    if(!_window)
        return;

    if(IsMaximized())
        glfwRestoreWindow(_window);
    else
        glfwMaximizeWindow(_window);
}

bool RetroFuturaGUI::Window::IsMaximized() const
{
    if(!_window)
        return false;

    return GLFW_TRUE == glfwGetWindowAttrib(_window, GLFW_MAXIMIZED);
}

void RetroFuturaGUI::Window::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadii = radii;
}

bool RetroFuturaGUI::Window::DockWindow(Window* window, const DockEdge edge)
{
    return dockWindowAt(window, edge, dockedWindows(edge).size());
}

bool RetroFuturaGUI::Window::dockWindowAt(Window* window, const DockEdge edge, const uSize index)
{
    if(!window)
        return false;

    if(!window->_window)
        return false;

    if(window == this)
        return false;

    //Docking previews and popups never dock
    if(WindowRole::Normal != _role || WindowRole::Normal != window->_role)
        return false;

    if(IsInTab() || window->IsInTab())
        return false;

    //A window has one host at most, the window whose edge sets its length and position, and never one docked to it, so every
    //group stays a tree. It can still border more windows: the ones docked to its own edges, and its neighbors on the host's edge.
    if(window->_dockHost)
        return false;

    if(isDockedTo(window))
        return false;

    glm::ivec2 size { 0 };
    glfwGetWindowSize(window->_window, &size.x, &size.y);

    window->_dockHost = this;
    window->_dockHostEdge = edge;
    //The length this window's edge takes over, so undocking can give it back
    window->_undockedLength = DockEdge::Left == edge || DockEdge::Right == edge ? size.y : size.x;

    std::list<Window*>& docked { dockedWindows(edge) };
    docked.insert(std::next(docked.begin(), static_cast<i64>(std::min(index, docked.size()))), window);

    layoutDockedWindows();
    return true;
}

void RetroFuturaGUI::Window::Undock()
{
    leaveDockHost();
    releaseDockedWindows();

    if(!_tabHost)
        return;

    _tabHost->RemoveWindow(this); //a free window again, where the tab's content was
}

bool RetroFuturaGUI::Window::IsDocked() const
{
    return _dockHost != nullptr
        || !_topDockedWindows.empty()
        || !_rightDockedWindows.empty()
        || !_bottomDockedWindows.empty()
        || !_leftDockedWindows.empty();
}

void RetroFuturaGUI::Window::SetDockingPadding(const f32 padding)
{
    _dockingPadding = std::max(padding, 0.0f);
    dockRoot()->layoutDockedWindows();
}

void RetroFuturaGUI::Window::SetNoCornerRadiiWhenDocked(const bool noCornerRadii)
{
    _noCornerRadiiWhenDocked = noCornerRadii;
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::FromGlfwWindow(GLFWwindow* glfwWindow)
{
    if(!glfwWindow)
        return nullptr;

    return static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
}

std::list<RetroFuturaGUI::Window*>& RetroFuturaGUI::Window::dockedWindows(const DockEdge edge)
{
    return const_cast<std::list<Window*>&>(std::as_const(*this).dockedWindows(edge));
}

const std::list<RetroFuturaGUI::Window*>& RetroFuturaGUI::Window::dockedWindows(const DockEdge edge) const
{
    switch(edge)
    {
        case DockEdge::Right:
            return _rightDockedWindows;
        case DockEdge::Bottom:
            return _bottomDockedWindows;
        case DockEdge::Left:
            return _leftDockedWindows;
        default:
            return _topDockedWindows;
    }
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::dockRoot()
{
    Window* root { this };

    while(root->_dockHost)
        root = root->_dockHost;

    return root;
}

bool RetroFuturaGUI::Window::isDockedTo(const Window* window) const
{
    for(const Window* host { _dockHost }; host; host = host->_dockHost)
    {
        if(host == window)
            return true;
    }

    return false;
}

void RetroFuturaGUI::Window::leaveDockHost()
{
    if(!_dockHost)
        return;

    Window* host { _dockHost };
    host->dockedWindows(_dockHostEdge).remove(this);
    _dockHost = nullptr;
    restoreUndockedLength();
    host->layoutDockedWindows(); //The windows left on that edge share all of it again, so they grow into the gap
}

void RetroFuturaGUI::Window::releaseDockedWindows()
{
    for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
    {
        std::list<Window*>& docked { dockedWindows(edge) };

        for(Window* window : docked)
        {
            if(!window)
                continue;

            window->_dockHost = nullptr;
            window->restoreUndockedLength();
        }

        docked.clear();
    }
}

void RetroFuturaGUI::Window::restoreUndockedLength()
{
    const i32 length { _undockedLength };
    _undockedLength = 0;

    if(!_window)
        return;

    if(0 >= length)
        return;

    if(isMinimizedOrMaximized())
        return;

    glm::ivec2 size { 0 };
    glfwGetWindowSize(_window, &size.x, &size.y);

    if(DockEdge::Left == _dockHostEdge || DockEdge::Right == _dockHostEdge)
        size.y = length;
    else
        size.x = length;

    glfwSetWindowSize(_window, size.x, size.y);
}

void RetroFuturaGUI::Window::layoutDockedWindows()
{
    if(!_window)
        return;

    //Its docked windows stay where they are rather than stacking off screen or over it, until it is restored
    if(isMinimizedOrMaximized())
    {
        _dockingLayoutPending = true;
        return;
    }

    const DockRect host { screenRect() };
    _dockedPosition = host._Position;
    _dockedSize = host._Size;

    for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
    {
        const std::list<Window*>& docked { dockedWindows(edge) };
        const uSize count { docked.size() };
        const i32 gap { edgeGap(edge, nullptr) };
        uSize index { 0 };

        for(Window* window : docked)
        {
            const uSize slot { index };
            ++index;

            if(!window)
                continue;

            if(!window->_window)
                continue;

            const DockRect placed { dockSlot(host, edge, slot, count, window->dockDepth(edge), gap) };
            window->setDockedGeometry(placed._Position, placed._Size);
            window->layoutDockedWindows();
        }
    }
}

RetroFuturaGUI::Window::DockRect RetroFuturaGUI::Window::screenRect() const
{
    DockRect rect {};

    if(!_window)
        return rect;

    glfwGetWindowPos(_window, &rect._Position.x, &rect._Position.y);
    glfwGetWindowSize(_window, &rect._Size.x, &rect._Size.y);
    return rect;
}

i32 RetroFuturaGUI::Window::edgeGap(const DockEdge edge, const Window* joining) const
{
    f32 padding { _dockingPadding };

    for(const Window* window : dockedWindows(edge))
    {
        if(!window)
            continue;

        padding = std::max(padding, window->_dockingPadding);
    }

    if(!joining)
        return static_cast<i32>(std::lround(padding));

    return static_cast<i32>(std::lround(std::max(padding, joining->_dockingPadding)));
}

i32 RetroFuturaGUI::Window::dockDepth(const DockEdge edge) const
{
    const DockRect rect { screenRect() };
    return DockEdge::Left == edge || DockEdge::Right == edge ? rect._Size.x : rect._Size.y;
}

RetroFuturaGUI::Window::DockRect RetroFuturaGUI::Window::dockSlot(const DockRect& host, const DockEdge edge, const uSize index, const uSize count, const i32 depth, const i32 gap) const
{
    //The windows on an edge share its whole length, minus the gaps between them, and keep their own depth away from it
    const bool alongHeight { DockEdge::Left == edge || DockEdge::Right == edge };
    const i32
        edgeLength { alongHeight ? host._Size.y : host._Size.x }, //Screen coordinates, so y runs downward.
        windowCount { std::max(static_cast<i32>(count), 1) },
        slot { static_cast<i32>(index) },
        share { (edgeLength - gap * (windowCount - 1)) / windowCount },
        offset { slot * (share + gap) },
        //The last one takes what the division left over
        length { std::max(windowCount - 1 == slot ? edgeLength - offset : share, _minWindowSpan) };

    switch(edge)
    {
        case DockEdge::Top:
            return DockRect { ._Position = host._Position + glm::ivec2(offset, -gap - depth), ._Size = glm::ivec2(length, depth) };
        case DockEdge::Right:
            return DockRect { ._Position = host._Position + glm::ivec2(host._Size.x + gap, offset), ._Size = glm::ivec2(depth, length) };
        case DockEdge::Bottom:
            return DockRect { ._Position = host._Position + glm::ivec2(offset, host._Size.y + gap), ._Size = glm::ivec2(length, depth) };
        default:
            return DockRect { ._Position = host._Position + glm::ivec2(-gap - depth, offset), ._Size = glm::ivec2(depth, length) };
    }
}

void RetroFuturaGUI::Window::setDockedGeometry(const glm::ivec2& position, const glm::ivec2& size)
{
    if(!_window)
        return;

    if(isMinimizedOrMaximized())
    {
        _dockingLayoutPending = true;
        return;
    }

    glm::ivec2
        currentPosition { 0 },
        currentSize { 0 };
    glfwGetWindowPos(_window, &currentPosition.x, &currentPosition.y);
    glfwGetWindowSize(_window, &currentSize.x, &currentSize.y);

    if(size != currentSize)
        glfwSetWindowSize(_window, size.x, size.y);

    if(position != currentPosition)
        glfwSetWindowPos(_window, position.x, position.y);

    _dockedPosition = position;
    _dockedSize = size;
}

void RetroFuturaGUI::Window::followDocking()
{
    if(!_window)
        return;

    if(!IsDocked())
        return;

    if(isMinimizedOrMaximized())
        return;

    Window* root { dockRoot() };

    if(_dockingLayoutPending)
    {
        _dockingLayoutPending = false;
        root->layoutDockedWindows();
        return;
    }

    glm::ivec2
        position { 0 },
        size { 0 };
    glfwGetWindowPos(_window, &position.x, &position.y);
    glfwGetWindowSize(_window, &size.x, &size.y);

    if(position == _dockedPosition && size == _dockedSize)
        return;

    //Moved or resized by something other than the docking layout: a window bar drag, an edge drag or the system. A docked
    //window that was moved, not resized, carries its group along, so the root moves by as much and the layout then places
    //every other window around it. A resize moves nothing: the layout already puts a resized window where its new size goes.
    if(root != this && size == _dockedSize)
    {
        //A minimized or maximized root can't move
        if(root->isMinimizedOrMaximized())
        {
            _dockedPosition = position;
            root->_dockingLayoutPending = true;
            return;
        }

        const glm::ivec2 shift { position - _dockedPosition };
        glm::ivec2 rootPosition { 0 };
        glfwGetWindowPos(root->_window, &rootPosition.x, &rootPosition.y);
        glfwSetWindowPos(root->_window, rootPosition.x + shift.x, rootPosition.y + shift.y);
    }

    root->layoutDockedWindows();
}

bool RetroFuturaGUI::Window::isMinimizedOrMaximized() const
{
    if(!_window)
        return false;

    return GLFW_TRUE == glfwGetWindowAttrib(_window, GLFW_ICONIFIED) || GLFW_TRUE == glfwGetWindowAttrib(_window, GLFW_MAXIMIZED);
}

glm::bvec4 RetroFuturaGUI::Window::squaredCorners() const
{
    glm::bvec4 squared { false };

    for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
    {
        if(dockedWindows(edge).empty())
            continue;

        squareEdgeCorners(squared, edge);
    }

    if(!_dockHost)
        return squared;

    //The edge facing the host lies against it from end to end
    squareEdgeCorners(squared, oppositeEdge(_dockHostEdge));

    //Neighbors on the same edge of the host sit before and after this window, across the docking padding
    const std::list<Window*>& neighbors { _dockHost->dockedWindows(_dockHostEdge) };
    const bool alongHeight { DockEdge::Left == _dockHostEdge || DockEdge::Right == _dockHostEdge };

    if(neighbors.front() != this)
        squareEdgeCorners(squared, alongHeight ? DockEdge::Top : DockEdge::Left);

    if(neighbors.back() != this)
        squareEdgeCorners(squared, alongHeight ? DockEdge::Bottom : DockEdge::Right);

    return squared;
}

glm::vec4 RetroFuturaGUI::Window::GetDrawnCornerRadii() const
{
    if(IsInTab())
        return _tabCornerRadii;

    glm::vec4 radii { _cornerRadii };

    if(!_noCornerRadiiWhenDocked)
        return radii;

    const glm::bvec4 squared { squaredCorners() };

    for(glm::length_t corner = 0; 4 > corner; ++corner)
    {
        if(squared[corner])
            radii[corner] = 0.0f;
    }

    return radii;
}

void RetroFuturaGUI::Window::applyCornerRadii()
{
    const glm::vec4 radii { GetDrawnCornerRadii() };

    if(radii == _appliedCornerRadii)
        return;

    _appliedCornerRadii = radii;
    setBackgroundCornerRadii(radii);
    setBorderCornerRadii(radii);
}

void RetroFuturaGUI::Window::squareEdgeCorners(glm::bvec4& squared, const DockEdge edge)
{
    switch(edge)
    {
        case DockEdge::Top:
            squared.x = true;
            squared.y = true;
        break;
        case DockEdge::Right:
            squared.y = true;
            squared.z = true;
        break;
        case DockEdge::Bottom:
            squared.z = true;
            squared.w = true;
        break;
        case DockEdge::Left:
            squared.w = true;
            squared.x = true;
        break;
        default:
        break;
    }
}

RetroFuturaGUI::DockEdge RetroFuturaGUI::Window::oppositeEdge(const DockEdge edge)
{
    switch(edge)
    {
        case DockEdge::Top:
            return DockEdge::Bottom;
        case DockEdge::Right:
            return DockEdge::Left;
        case DockEdge::Bottom:
            return DockEdge::Top;
        default:
            return DockEdge::Right;
    }
}

void RetroFuturaGUI::Window::BeginDrag()
{
    if(!_window)
        return;

    _isDragged = true;
    _dockSnap = {};
    glfwGetWindowPos(_window, &_dragStartPosition.x, &_dragStartPosition.y);
}

void RetroFuturaGUI::Window::EndDrag()
{
    if(!_isDragged)
        return;

    _isDragged = false;
    hideDockPreview();

    const DockSnap snap { _dockSnap };
    _dockSnap = {};

    if(!snap._Target)
        return;

    if(std::ranges::find(allWindows(), snap._Target) == allWindows().end())
        return;

    if(!snap._Tab)
    {
        snap._Target->dockWindowAt(this, snap._Edge, snap._Index);
        return;
    }

    if(!isRegisteredWindowTab(snap._Tab))
        return;

    snap._Tab->hideDropPreview();
    snap._Tab->AddWindow(this);
}

void RetroFuturaGUI::Window::updateDockSnap()
{
    if(!_isDragged)
        return;

    WindowTab* previousTab { _dockSnap._Tab };
    _dockSnap = findDockSnap();

    if(previousTab && previousTab != _dockSnap._Tab && isRegisteredWindowTab(previousTab))
        previousTab->hideDropPreview();

    if(!_dockSnap._Target)
    {
        hideDockPreview();
        return;
    }

    if(_dockSnap._Tab)
    {
        //get the content area it would fill, and a field with its title at the end of the strip
        const glm::ivec4 content { _dockSnap._Tab->contentScreenRect() };
        _dockSnap._Tab->showDropPreview(_windowTitle);
        showDockPreview(DockRect 
                            { ._Position = glm::ivec2(content.x, content.y), ._Size = glm::ivec2(content.z, content.w) },
                                _dockSnap._Tab->GetContentCornerRadii()
                        );
        return;
    }

    //The slot this window would take, with the windows already on that edge making room for it
    const DockRect host { _dockSnap._Target->screenRect() };
    const uSize count { _dockSnap._Target->dockedWindows(_dockSnap._Edge).size() + 1 };
    const i32 gap { _dockSnap._Target->edgeGap(_dockSnap._Edge, this) };
    showDockPreview(_dockSnap._Target->dockSlot(host, _dockSnap._Edge, _dockSnap._Index, count, dockDepth(_dockSnap._Edge), gap), cornerRadiiDockedAt(_dockSnap._Edge));
}

RetroFuturaGUI::Window::DockSnap RetroFuturaGUI::Window::findDockSnap() const
{
    if(!_window)
        return {};

    if(WindowRole::Normal != _role)
        return {}; //popups never dock

    //Only a window docked to nothing docks by dragging. Dragging a docked one carries its whole group along instead.
    if(_dockHost)
        return {};

    if(isMinimizedOrMaximized())
        return {};

    const DockRect dragged { screenRect() };

    //A click on the title is no drag: the window has to have moved
    if(dragged._Position == _dragStartPosition)
        return {};

    const DockSnap tabSnap { findTabSnap() }; //Over a WindowTab's strip, dropping into the tab wins over docking to an edge!

    if(tabSnap._Tab)
        return tabSnap;

    DockSnap snap {};
    i32 closest { _snapDistance + 1 };

    for(Window* target : allWindows())
    {
        if(!target)
            continue;

        if(target == this)
            continue;

        if(!target->_window)
            continue;

        //A window of its own group would make the group a loop
        if(target->isDockedTo(this))
            continue;

        if(target->isMinimizedOrMaximized())
            continue;

        if(GLFW_FALSE == glfwGetWindowAttrib(target->_window, GLFW_VISIBLE))
            continue;

        const DockRect host { target->screenRect() };

        for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
        {
            //Measured to where it would really sit, with the gap it would get there
            const i32 distance { dockDistance(dragged, host, target->edgeGap(edge, this), edge) };

            if(distance >= closest)
                continue;

            if(0 >= dockOverlap(dragged, host, edge))
                continue;

            closest = distance;
            snap = DockSnap { ._Target = target, ._Edge = edge, ._Index = target->dockIndexFor(dragged, edge) };
        }
    }

    return snap;
}

RetroFuturaGUI::Window::DockSnap RetroFuturaGUI::Window::findTabSnap() const
{
    if(!_window)
        return {};

    if(holdsTabbedWindows() || IsDocked())
        return {}; //a window carrying docked windows along would have to leave them behind

    i32
        cursorX { 0 },
        cursorY { 0 };

    if(!getCursorPosition(cursorX, cursorY))
        return {};

    const glm::ivec2 cursor { screenRect()._Position + glm::ivec2(cursorX, cursorY) };

    for(Window* target : allWindows())
    {
        if(!target)
            continue;

        if(target == this)
            continue;

        if(!target->_window)
            continue;

        if(target->IsInTab() || target->isDockedTo(this))
            continue; //a window of this one's group moves along with it

        if(GLFW_TRUE == glfwGetWindowAttrib(target->_window, GLFW_ICONIFIED))
            continue;

        if(GLFW_FALSE == glfwGetWindowAttrib(target->_window, GLFW_VISIBLE))
            continue;

        for(WindowTab* tab : target->_windowTabs)
        {
            if(!tab)
                continue;

            if(!tab->isStripUnder(cursor))
                continue;

            return DockSnap { ._Target = target, ._Edge = DockEdge::Top, ._Index = tab->GetWindowCount(), ._Tab = tab };
        }
    }

    return {};
}

uSize RetroFuturaGUI::Window::dockIndexFor(const DockRect& dragged, const DockEdge edge) const
{
    //Along the edge: y beside the window, x above or below it
    const glm::length_t axis { DockEdge::Left == edge || DockEdge::Right == edge ? 1 : 0 };
    const i32 draggedMiddle { dragged._Position[axis] + dragged._Size[axis] / 2 };
    uSize index { 0 };

    for(const Window* window : dockedWindows(edge))
    {
        if(!window)
            continue;

        const DockRect docked { window->screenRect() };

        if(docked._Position[axis] + docked._Size[axis] / 2 < draggedMiddle)
            ++index;
    }

    return index;
}

void RetroFuturaGUI::Window::showDockPreview(const DockRect& slot, const glm::vec4& cornerRadii)
{
    if(0 >= slot._Size.x || 0 >= slot._Size.y)
        return;

    if(!_dockPreview)
    {
        _dockPreview = std::unique_ptr<Window>(new Window("DockPreview", slot._Size.x, slot._Size.y, WindowRole::DockPreview));
        _dockPreview->SetWindowTitle("Dock preview");
        _dockPreview->SetBackgroundColor(glm::vec4(0.3f, 0.55f, 1.0f, 0.25f), ColorState::Enabled);
        _dockPreview->SetBorderWidth(2.0f);
        _dockPreview->SetBorderColor(glm::vec4(0.45f, 0.7f, 1.0f, 0.85f), ColorState::Enabled);
    }

    if(!_dockPreview->_window)
        return;

    _dockPreview->SetCornerRadii(cornerRadii);

    const DockRect current { _dockPreview->screenRect() };

    if(slot._Size != current._Size)
        glfwSetWindowSize(_dockPreview->_window, slot._Size.x, slot._Size.y);

    if(slot._Position != current._Position)
        glfwSetWindowPos(_dockPreview->_window, slot._Position.x, slot._Position.y);

    _dockPreview->Draw(); //Drawn before it shows, so it never shows a frame it hasn't drawn

    if(GLFW_FALSE == glfwGetWindowAttrib(_dockPreview->_window, GLFW_VISIBLE))
        glfwShowWindow(_dockPreview->_window);
}

glm::vec4 RetroFuturaGUI::Window::cornerRadiiDockedAt(const DockEdge edge) const
{
    glm::vec4 radii { _cornerRadii };

    if(!_noCornerRadiiWhenDocked)
        return radii;

    glm::bvec4 squared { false };
    squareEdgeCorners(squared, oppositeEdge(edge));

    for(glm::length_t corner = 0; 4 > corner; ++corner)
    {
        if(squared[corner])
            radii[corner] = 0.0f;
    }

    return radii;
}

void RetroFuturaGUI::Window::hideDockPreview()
{
    if(!_dockPreview)
        return;

    if(!_dockPreview->_window)
        return;

    if(GLFW_FALSE == glfwGetWindowAttrib(_dockPreview->_window, GLFW_VISIBLE))
        return;

    glfwHideWindow(_dockPreview->_window);
}

i32 RetroFuturaGUI::Window::dockDistance(const DockRect& dragged, const DockRect& host, const i32 gap, const DockEdge edge)
{
    switch(edge)
    {
        case DockEdge::Top:
            return std::abs(dragged._Position.y + dragged._Size.y - (host._Position.y - gap));
        case DockEdge::Right:
            return std::abs(dragged._Position.x - (host._Position.x + host._Size.x + gap));
        case DockEdge::Bottom:
            return std::abs(dragged._Position.y - (host._Position.y + host._Size.y + gap));
        default:
            return std::abs(dragged._Position.x + dragged._Size.x - (host._Position.x - gap));
    }
}

i32 RetroFuturaGUI::Window::dockOverlap(const DockRect& dragged, const DockRect& host, const DockEdge edge)
{
    //Along the edge: y beside the host, x above or below it
    const glm::length_t axis { DockEdge::Left == edge || DockEdge::Right == edge ? 1 : 0 };
    const i32
        start { std::max(dragged._Position[axis], host._Position[axis]) },
        end { std::min(dragged._Position[axis] + dragged._Size[axis], host._Position[axis] + host._Size[axis]) };

    return end - start;
}

std::vector<RetroFuturaGUI::Window*>& RetroFuturaGUI::Window::allWindows()
{
    //Deliberately never freed: a window owned by a static is destroyed after every other static, and still takes itself off
    static std::vector<Window*>* windows { new std::vector<Window*>() };
    return *windows;
}

bool RetroFuturaGUI::Window::PushPopup(std::unique_ptr<IPopup> popup)
{
    if(!popup)
        return false;

    //The window holding the tab takes the popups, so they block the tab strip too
    if(IsInTab())
    {
        Window* host { tabHostWindow() };

        if(!host)
            return false;

        return host->PushPopup(std::move(popup));
    }

    //A popup's window keeps no stack of its own: a popup opened from it goes on top of the stack it is on
    if(WindowRole::Popup == _role)
    {
        if(!_popupHost)
            return false;

        return _popupHost->PushPopup(std::move(popup));
    }

    if(!_window)
        return false;

    Window* popupWindow { popup->GetWindow() };

    if(!popupWindow)
        return false;

    if(!popupWindow->_window)
        return false;

    if(popupWindow->_popupHost)
        return false; //Already on a stack

    //It opens over the current top, which owns it natively
    Window* owner { topPopupWindow() };
    popupWindow->_popupHost = this;
    popupWindow->_ignoreHeldPress = true; //the click that opened it may still be down!
    popupWindow->_showAfterDraw = true;
    popupWindow->setNativeOwner(owner);
    popupWindow->centerOver(owner);

    popup->_isOnStack = true;
    _popups.push_back(std::move(popup));
    _popups.back()->restoreOpenerContext();
    return true;
}

bool RetroFuturaGUI::Window::HasPopups() const
{
    return !_popups.empty();
}

bool RetroFuturaGUI::Window::IsBlockedByPopup() const
{
    if(IsInTab())
    {
        const Window* host { tabHostWindow() };

        if(!host)
            return false;

        return host->IsBlockedByPopup();
    }

    if(!_popups.empty())
        return true;

    if(!_popupHost)
        return false;

    //A popup's window is blocked unless it is the top of the stack
    return _popupHost->topPopupWindow() != this;
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

void RetroFuturaGUI::Window::drawPopups()
{
    //By index: a button in a popup may open another popup while this loop runs, and push_back would invalidate an iterator
    for(uSize index = 0; index < _popups.size(); ++index)
    {
        IPopup* popup { _popups[index].get() };

        if(!popup)
            continue;

        Window* popupWindow { popup->GetWindow() };

        if(!popupWindow)
            continue;

        popupWindow->Draw(); //shows itself once it has drawn its first frame
    }

    closeTopPopup();
}

void RetroFuturaGUI::Window::closeTopPopup()
{
    if(_popups.empty())
        return;

    Window* topWindow { topPopupWindow() };

    if(topWindow == this)
        return;

    if(!topWindow->WindowShouldClose())
        return;

    IPopup* top { _popups.back().get() };

    if(!top)
        return;

    //Closed by the system rather than the popup, which may keep its window open
    if(!top->_isCloseRequested && !top->allowsSystemClose())
    {
        glfwSetWindowShouldClose(topWindow->_window, GLFW_FALSE);
        return;
    }

    std::unique_ptr<IPopup> closing { std::move(_popups.back()) }; //Off the stack before its result goes out, so a slot that opens a follow-up popup puts it on top
    _popups.pop_back();
    topPopupWindow()->_ignoreHeldPress = true; //The window below takes the mouse again, but not the press on the button that closed the popup

    if(!closing)
        return;

    closing->emitClosed();
    closing.reset();

    //Destroying the popup's window left no context current
    glfwMakeContextCurrent(_window);
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::topPopupWindow()
{
    if(_popups.empty())
        return this;

    IPopup* top { _popups.back().get() };

    if(!top)
        return this;

    Window* topWindow { top->GetWindow() };

    if(!topWindow)
        return this;

    return topWindow;
}

void RetroFuturaGUI::Window::refuseCloseWhileBlocked()
{
    if(!_window)
        return;

    if(!IsBlockedByPopup())
        return;

    glfwSetWindowShouldClose(_window, GLFW_FALSE);
}

void RetroFuturaGUI::Window::centerOver(const Window* window)
{
    if(!window)
        return;

    if(!_window)
        return;

    if(!window->_window)
        return;

    const DockRect
        over { window->screenRect() },
        own { screenRect() };
    const glm::ivec2 position { over._Position + (over._Size - own._Size) / 2 };
    glfwSetWindowPos(_window, position.x, position.y);
}

void RetroFuturaGUI::Window::setNativeOwner(const Window* owner)
{
    if(!owner)
        return;

    if(!_window)
        return;

    if(!owner->_window)
        return;

#if defined(TARGET_PLATFORM_LINUX)
    PlatformBridge::NativeWindow::SetOwner(glfwGetX11Display(), glfwGetX11Window(_window), glfwGetX11Window(owner->_window));
#elif defined(TARGET_PLATFORM_WINDOWS)
    PlatformBridge::NativeWindow::SetOwner(glfwGetWin32Window(_window), glfwGetWin32Window(owner->_window));
#endif
}

bool RetroFuturaGUI::Window::IsInTab() const
{
    return _tabHost != nullptr;
}

const std::string& RetroFuturaGUI::Window::GetWindowTitle() const
{
    return _windowTitle;
}

GLFWwindow* RetroFuturaGUI::Window::GetKeyboardWindow() const
{
    const Window* host { tabHostWindow() };

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

void RetroFuturaGUI::Window::RequestCursorIcon(GLFWwindow* glfwWindow, const PlatformBridge::CursorIcon icon)
{
    //The window keys arrive at is the one on screen: the host's while this one sits in a WindowTab
    Window* window { FromGlfwWindow(KeyboardWindowOf(glfwWindow)) };

    if(!window)
        return;

    window->_requestedCursorIcon = icon;
}

glm::ivec2 RetroFuturaGUI::Window::DrawOriginOf(GLFWwindow* glfwWindow)
{
    const Window* window { FromGlfwWindow(glfwWindow) };

    if(!window)
        return glm::ivec2(0);

    return window->_drawOrigin;
}

bool RetroFuturaGUI::Window::enterTab(WindowTab* tab)
{
    if(!tab)
        return false;

    if(!_window)
        return false;

    const Window* host { tab->GetHostWindow() };

    if(!host)
        return false;

    if(WindowRole::Normal != _role || host == this)
        return false;

    if(_tabHost == tab)
        return false;

    if(holdsTabbedWindows() || host->IsInTab())
        return false;

    Undock(); //Leaves its docking group, or the tab it was in, and lets go of the windows docked to it

    if(isMinimizedOrMaximized())
        glfwRestoreWindow(_window);

    if(_dockSnap._Tab && isRegisteredWindowTab(_dockSnap._Tab))
        _dockSnap._Tab->hideDropPreview(); //A drag in progress ends here, since the window no longer moves on its own

    _isDragged = false;
    _followsCursor = false;
    _dockSnap = {};
    hideDockPreview();

    glfwGetWindowSize(_window, &_untabbedSize.x, &_untabbedSize.y);
    _tabHost = tab;
    _showAfterDraw = false;
    glfwHideWindow(_window);
    UpdateLayout(); //the scenes hidden in a tab give their strips back
    return true;
}

void RetroFuturaGUI::Window::leaveTab(const glm::ivec2& position)
{
    if(!_tabHost)
        return;

    _tabHost = nullptr;
    _tabCornerRadii = glm::vec4(0.0f);

    if(!_window)
        return;

    const glm::ivec2 size { 0 < _untabbedSize.x && 0 < _untabbedSize.y ? _untabbedSize : glm::ivec2(_width, _height) };
    glfwSetWindowSize(_window, size.x, size.y);
    glfwSetWindowPos(_window, position.x, position.y);

    //Shows once it has drawn a frame of its own, so it never shows the picture its native window held before it went into the
    //tab. Its own context's viewport is still the one from back then too, so the next frame sets it again.
    _showAfterDraw = true;
    _lastSize = glm::ivec2(-1);
    UpdateLayout(); //the scenes hidden in a tab take their strips again
}

void RetroFuturaGUI::Window::drawInHost(const ScreenRect& contentArea, const glm::ivec2& screenPosition, const glm::vec4& cornerRadii)
{
    if(!_window)
        return;

    const Window* host { tabHostWindow() };

    if(!host)
        return;

    //Its native window is hidden and never under the cursor, so the mouse reaches it whenever it reaches the window drawing it
    _hasPointer = host->_hasPointer;

    //Left, bottom, width and height in the drawing window's framebuffer
    const glm::ivec4 area
    {
        static_cast<i32>(std::lround(contentArea._Origin.x)),
        static_cast<i32>(std::lround(contentArea._Origin.y)),
        std::max(static_cast<i32>(std::lround(contentArea._Extent.x)), 1),
        std::max(static_cast<i32>(std::lround(contentArea._Extent.y)), 1)
    };

    //The hidden native window lies right under the content area, so its widgets read the cursor as they do on the window itself
    setDockedGeometry(screenPosition, glm::ivec2(area.z, area.w));
    _tabCornerRadii = cornerRadii;
    updateSize();

    //Once the button is up, a press held from before this window took the mouse is over
    if(!PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left))
        _ignoreHeldPress = false;

    //The drawing window's viewport and scissor box, given back once this window is drawn
    GLint
        viewport[4] {},
        scissorBox[4] {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
    const bool scissorWasEnabled { GL_TRUE == glIsEnabled(GL_SCISSOR_TEST) };

    //Everything lands in the content area, depth clears included, and an active clip stays in force
    i32
        clipLeft { area.x },
        clipBottom { area.y },
        clipRight { area.x + area.z },
        clipTop { area.y + area.w };

    if(scissorWasEnabled)
    {
        clipLeft = std::max(clipLeft, scissorBox[0]);
        clipBottom = std::max(clipBottom, scissorBox[1]);
        clipRight = std::min(clipRight, scissorBox[0] + scissorBox[2]);
        clipTop = std::min(clipTop, scissorBox[1] + scissorBox[3]);
    }

    glViewport(area.x, area.y, area.z, area.w);
    glEnable(GL_SCISSOR_TEST);
    glScissor(clipLeft, clipBottom, std::max(clipRight - clipLeft, 0), std::max(clipTop - clipBottom, 0));
    _drawOrigin = glm::ivec2(area.x, area.y);

    //The drawing window's own tooltip and open lists still draw last in its frame
    Tooltip::setShownAside();
    std::vector<IDropDown*> hostDropDowns { IDropDown::takeShown() };
    glClear(GL_DEPTH_BUFFER_BIT);
    drawContent();
    glClear(GL_DEPTH_BUFFER_BIT); //the drawing window's widgets after the WindowTab aren't tested against this window's content
    IDropDown::restoreShown(std::move(hostDropDowns));
    Tooltip::restoreShownFromAside();

    _drawOrigin = glm::ivec2(0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glScissor(scissorBox[0], scissorBox[1], scissorBox[2], scissorBox[3]);

    if(!scissorWasEnabled)
        glDisable(GL_SCISSOR_TEST);
}

void RetroFuturaGUI::Window::beginCursorDrag(const glm::ivec2& grabPoint)
{
    if(!_window)
        return;

    _cursorGrabPoint = grabPoint;
    _followsCursor = true;
    _ignoreHeldPress = true; //the press that pulled it out is still down, and isn't a click on whatever lies under it now
    BeginDrag();
}

void RetroFuturaGUI::Window::followCursor()
{
    if(!_followsCursor)
        return;

    if(!_window)
        return;

    if(!PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left))
    {
        _followsCursor = false;
        EndDrag(); //docks it, or drops it into a tab, where its preview showed
        return;
    }

    i32
        cursorX { 0 },
        cursorY { 0 };

    if(!getCursorPosition(cursorX, cursorY))
        return;

    //How far the cursor has moved from the point it holds
    const glm::ivec2 slip { glm::ivec2(cursorX, cursorY) - _cursorGrabPoint };

    if(0 == slip.x && 0 == slip.y)
        return;

    i32
        windowX { 0 },
        windowY { 0 };
    glfwGetWindowPos(_window, &windowX, &windowY);
    glfwSetWindowPos(_window, windowX + slip.x, windowY + slip.y);
}

bool RetroFuturaGUI::Window::isSceneShown(const Scene* scene) const
{
    if(!scene)
        return false;

    if(!scene->IsActive())
        return false;

    if(!IsInTab())
        return true;

    return !scene->IsHiddenInTab();
}

bool RetroFuturaGUI::Window::holdsTabbedWindows() const
{
    for(const WindowTab* tab : _windowTabs)
    {
        if(!tab)
            continue;

        if(0 < tab->GetWindowCount())
            return true;
    }

    return false;
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::tabHostWindow() const
{
    if(!_tabHost)
        return nullptr;

    return _tabHost->GetHostWindow();
}

bool RetroFuturaGUI::Window::isRegisteredWindowTab(const WindowTab* tab)
{
    if(!tab)
        return false;

    for(const Window* window : allWindows())
    {
        if(!window)
            continue;

        if(std::ranges::find(window->_windowTabs, tab) != window->_windowTabs.end())
            return true;
    }

    return false;
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