#include "Window.hpp"
#include "WindowTab.hpp"
#include "ScissorClip.hpp"
#include "PlatformBridge.hpp"
#include "IDropDown.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

bool RetroFuturaGUI::Window::IsInTab() const
{
    return _tabHost != nullptr;
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

void RetroFuturaGUI::Window::interactInHost(const ScreenRect& contentArea, const glm::ivec2& screenPosition, const glm::vec4& cornerRadii)
{
    if(!_window)
        return;

    const Window* host { getTabHostWindow() };

    if(!host)
        return;

    //Its native window is hidden and never under the cursor, so the mouse reaches it whenever it reaches the window drawing it
    _hasPointer = host->_hasPointer;
    placeInHost(contentArea, screenPosition, cornerRadii);

    //Once the button is up, a press held from before this window took the mouse is over
    if(!PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left))
        _ignoreHeldPress = false;

    interactContent();
}

void RetroFuturaGUI::Window::drawInHost(const ScreenRect& contentArea, const glm::ivec2& screenPosition, const glm::vec4& cornerRadii)
{
    if(!_window)
        return;

    const glm::ivec4 area { placeInHost(contentArea, screenPosition, cornerRadii) }; //again, in case the WindowTab moved since it interacted

    //The drawing window's viewport, given back once this window is drawn
    GLint viewport[4] {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(area.x, area.y, area.z, area.w);
    _drawOrigin = glm::ivec2(area.x, area.y);

    //The drawing window's own open lists still draw last in its frame. Its tooltip and context menu wait under its own window.
    std::vector<IDropDown*> hostDropDowns { IDropDown::takeShown() };

    {
        //Everything lands in the content area, depth clears included, and an active clip stays in force
        const ScissorClip clip { area };
        glClear(GL_DEPTH_BUFFER_BIT);
        drawContent();
        glClear(GL_DEPTH_BUFFER_BIT); //the drawing window's widgets after the WindowTab aren't tested against this window's content
    }

    IDropDown::restoreShown(std::move(hostDropDowns));

    _drawOrigin = glm::ivec2(0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

glm::ivec4 RetroFuturaGUI::Window::placeInHost(const ScreenRect& contentArea, const glm::ivec2& screenPosition, const glm::vec4& cornerRadii)
{
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
    return area;
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

RetroFuturaGUI::Window* RetroFuturaGUI::Window::getTabHostWindow() const
{
    if(!_tabHost)
        return nullptr;

    return _tabHost->GetHostWindow();
}

bool RetroFuturaGUI::Window::isRegisteredWindowTab(const WindowTab* tab)
{
    if(!tab)
        return false;

    for(const Window* window : getAllWindows())
    {
        if(!window)
            continue;

        if(std::ranges::find(window->_windowTabs, tab) != window->_windowTabs.end())
            return true;
    }

    return false;
}
