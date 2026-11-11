#include "Window.hpp"
#include "WindowTab.hpp"
#include "ScissorClip.hpp"
#include "PlatformBridge.hpp"
#include "IDropDown.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

bool RetroFuturaGUI::Window::IsInTab() const
{
    return _tabHost != nullptr;
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

    //Tabs nest, but a window can't go into a tab inside itself
    if(host->isInTabsOf(this))
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
    markLayoutChanged();
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
    markLayoutChanged();
}

void RetroFuturaGUI::Window::interactInHost(const TabPlacement& placement)
{
    if(!_window)
        return;

    const Window* host { getTabHostWindow() };

    if(!host)
        return;

    //Its native window is hidden and never under the cursor, so the mouse reaches it whenever it reaches the window drawing it
    _hasPointer = host->_hasPointer;
    placeInHost(placement);

    //Once the button is up, a press held from before this window took the mouse is over
    if(!PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left))
        _ignoreHeldPress = false;

    //Its widgets take the frame they are drawn in, so the pointer meets them where they show.
    const Shader::FrameScope frame { _isTurnedInTab ? _turnedContentFrame : glm::mat4(1.0f), Shader::FrameComposition::Instead };
    interactContent();
}

void RetroFuturaGUI::Window::drawInHost(const TabPlacement& placement)
{
    if(!_window)
        return;

    const glm::ivec4 area { placeInHost(placement) }; //again, in case the WindowTab moved since it interacted

    if(_isTurnedInTab)
    {
        drawTurnedInHost();
        return;
    }

    //The drawing window's viewport, given back once this window is drawn
    GLint viewport[4] {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(area.x, area.y, area.z, area.w);

    //The drawing window's own open lists still draw last in its frame. Its tooltip and context menu wait under its own window.
    std::vector<IDropDown*> hostDropDowns { IDropDown::takeShown() };

    {
        //Everything lands in the content area, depth clears included, and an active clip stays in force
        const ScissorClip clip { area };
        const Shader::FrameScope flat { glm::mat4(1.0f), Shader::FrameComposition::Instead }; //its content lies in its own space.
        const Shader::UnclippedScope unclipped {}; //likewise, so the host's clip planes don't reach into it.
        const Transitions::InstantScope instant { _isHidden }; //a tab just selected shows what changed meanwhile at once
        const Shader::DepthClearScope depthClear { nullptr }; //its own, whole area
        glClear(GL_DEPTH_BUFFER_BIT);
        drawContent();
        glClear(GL_DEPTH_BUFFER_BIT); //the drawing window's widgets after the WindowTab aren't tested against this window's content
    }

    _isDrawnSinceUpdate = true;

    IDropDown::restoreShown(std::move(hostDropDowns));

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void RetroFuturaGUI::Window::drawTurnedInHost()
{
    if(!_projection)
        return;

    //The drawing window's own open lists still draw last in its frame
    std::vector<IDropDown*> hostDropDowns { IDropDown::takeShown() };

    {
        //Laid onto the turned area in the drawing window's viewport, cut to the area by planes, since no scissor box can turn
        const glm::vec2 resolution { _projection->GetResolution() };
        const Shader::FrameScope frame { _turnedContentFrame, Shader::FrameComposition::Instead };
        const Shader::UnclippedScope unclipped {}; //the host's planes lie in another space.
        const Shader::ClipScope clip { glm::vec4(0.0f, 0.0f, resolution.x, resolution.y), 0.0f, *_projection };
        const Transitions::InstantScope instant { _isHidden }; //a tab just selected shows what changed meanwhile at once
        const Shader::DepthClearScope depthClear { [this] { resetTurnedContentDepth(); } }; //a clear would reach past the turned area
        resetTurnedContentDepth();
        drawContent();
        resetTurnedContentDepth(); //the drawing window's widgets after the WindowTab aren't tested against this window's content
    }

    _isDrawnSinceUpdate = true;
    IDropDown::restoreShown(std::move(hostDropDowns));
}

void RetroFuturaGUI::Window::resetTurnedContentDepth()
{
    if(!_projection)
        return;

    if(!_turnedDepthReset)
    {
        _turnedDepthReset = std::make_unique<Plane>(_projection.get());
        _turnedDepthReset->SetColors(std::array { glm::vec4(1.0f) }); //opaque, so no fragment of it is left out.
    }

    const glm::vec2 resolution { _projection->GetResolution() };
    _turnedDepthReset->SetSize(resolution);
    _turnedDepthReset->SetPosition(glm::vec3(resolution * 0.5f, 0.0f));

    //Into the depth buffer only, at the far end whatever lies there, so the content draws over it
    GLint depthFunction { GL_LESS };
    GLfloat depthRange[2] { 0.0f, 1.0f };
    GLboolean colorMask[4] { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
    glGetFloatv(GL_DEPTH_RANGE, depthRange);
    glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthFunc(GL_ALWAYS);
    glDepthRange(1.0, 1.0);

    {
        const Shader::OpacityScope opaque { 1.0f, Shader::FrameComposition::Instead };
        _turnedDepthReset->Draw();
    }

    glDepthRange(depthRange[0], depthRange[1]);
    glDepthFunc(static_cast<GLenum>(depthFunction));
    glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
}

glm::ivec4 RetroFuturaGUI::Window::placeInHost(const TabPlacement& placement)
{
    const ScreenRect& contentArea { placement._ContentArea };

    //A host in a tab itself is drawn into its own host's framebuffer, where its content area starts at its own origin
    const Window* host { getTabHostWindow() };
    const glm::ivec2 hostOrigin { host && host->IsInTab() ? host->_hostFramebufferOrigin : glm::ivec2(0) };

    //Left, bottom, width and height in the drawing window's framebuffer
    const glm::ivec4 area
    {
        hostOrigin.x + static_cast<i32>(std::lround(contentArea._Origin.x)),
        hostOrigin.y + static_cast<i32>(std::lround(contentArea._Origin.y)),
        std::max(static_cast<i32>(std::lround(contentArea._Extent.x)), 1),
        std::max(static_cast<i32>(std::lround(contentArea._Extent.y)), 1)
    };
    _hostFramebufferOrigin = glm::ivec2(area.x, area.y);

    //The hidden native window lies right under the content area, so its widgets read the cursor as they do on the window itself
    setDockedGeometry(placement._ScreenPosition, glm::ivec2(area.z, area.w));
    _tabCornerRadii = placement._CornerRadii;
    updateSize();

    //Turned, its widgets read the cursor through the host instead, and draw through the frame laying them onto the area
    _isTurnedInTab = !placement._IsFlat;

    if(_isTurnedInTab)
        _turnedContentFrame = getTurnedContentFrame(placement);

    return area;
}

glm::mat4 RetroFuturaGUI::Window::getTurnedContentFrame(const TabPlacement& placement) const
{
    if(!_projection)
        return glm::mat4(1.0f);

    const glm::mat4& contentProjection { _projection->GetProjectionMatrix() };
    const glm::vec2
        origin { placement._ContentArea._Origin },
        extent { glm::max(placement._ContentArea._Extent, glm::vec2(1.0f)) };

    //The content's clip space laid flat onto the area: -1 .. 1 across it, depth 0, its w kept for a content in perspective
    glm::mat4 flatten { 0.0f };
    flatten[0][0] = extent.x * 0.5f;
    flatten[1][1] = extent.y * 0.5f;
    flatten[3] = glm::vec4(extent * 0.5f, 0.0f, 1.0f);
    const glm::mat4 areaView { placement._HostView * glm::translate(glm::mat4(1.0f), glm::vec3(origin, placement._Depth)) };
    glm::mat4 laidOn { areaView * flatten * contentProjection };

    //Lying flat, the content would lose its stacking. Its own depth goes in instead, scaled so it stays in the view's depth range
    //at the area's nearest corner. One pixel's fragments share their place on the area, so their order is the content's own
    f32 nearestW { std::numeric_limits<f32>::max() };

    for(const glm::vec2 corner : { glm::vec2(0.0f), glm::vec2(extent.x, 0.0f), extent, glm::vec2(0.0f, extent.y) })
        nearestW = std::min(nearestW, (areaView * glm::vec4(corner, 0.0f, 1.0f)).w);

    const f32 depthScale { 0.999f * std::max(nearestW, 1e-4f) };

    for(glm::length_t column = 0; 4 > column; ++column)
        laidOn[column][2] = depthScale * contentProjection[column][2];

    return glm::inverse(contentProjection) * laidOn;
}

glm::vec2 RetroFuturaGUI::Window::screenToWindowPoint(const glm::ivec2& screenPoint) const
{
    const glm::vec2 resolution { _projection ? _projection->GetResolution() : glm::vec2(1.0f) };
    const Window* host { getTabHostWindow() };

    //The host's point, in the host's pixels, is the same point of the view in this window's
    if(_isTurnedInTab && host && host->_projection)
        return host->screenToWindowPoint(screenPoint) / glm::max(host->_projection->GetResolution(), glm::vec2(1.0f)) * resolution;

    glm::ivec2 windowPosition { 0 };

    if(_window)
        glfwGetWindowPos(_window, &windowPosition.x, &windowPosition.y);

    const glm::ivec2 local { screenPoint - windowPosition };
    return glm::vec2(static_cast<f32>(local.x), resolution.y - static_cast<f32>(local.y));
}

glm::ivec2 RetroFuturaGUI::Window::windowPointToScreen(const glm::vec2& windowPoint) const
{
    const glm::vec2 resolution { _projection ? _projection->GetResolution() : glm::vec2(1.0f) };
    const Window* host { getTabHostWindow() };

    if(_isTurnedInTab && host && host->_projection)
        return host->windowPointToScreen(windowPoint / glm::max(resolution, glm::vec2(1.0f)) * host->_projection->GetResolution());

    glm::ivec2 windowPosition { 0 };

    if(_window)
        glfwGetWindowPos(_window, &windowPosition.x, &windowPosition.y);

    return windowPosition + glm::ivec2(static_cast<i32>(std::lround(windowPoint.x)), static_cast<i32>(std::lround(resolution.y - windowPoint.y)));
}

bool RetroFuturaGUI::Window::readTurnedTabPointer(GLFWwindow* glfwWindow, PointerState& pointer)
{
    const Window* window { FromGlfwWindow(glfwWindow) };

    if(!window)
        return false;

    if(!window->_isTurnedInTab)
        return false;

    //Up to the first window that reads the cursor itself: one not in a tab, or flat in one, its native window under the area
    const Window* reader { window };

    while(reader && reader->_isTurnedInTab)
        reader = reader->getTabHostWindow();

    if(!reader)
        return false;

    i32
        cursorX { 0 },
        cursorY { 0 };

    pointer._HasPosition = reader->getCursorPosition(cursorX, cursorY);

    if(!reader->_window)
        return true;

    glm::ivec2 readerPosition { 0 };
    glfwGetWindowPos(reader->_window, &readerPosition.x, &readerPosition.y);
    pointer._WindowPosition = window->screenToWindowPoint(readerPosition + glm::ivec2(cursorX, cursorY));
    return true;
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

RetroFuturaGUI::Window* RetroFuturaGUI::Window::getTabHostWindow() const
{
    if(!_tabHost)
        return nullptr;

    return _tabHost->GetHostWindow();
}

bool RetroFuturaGUI::Window::isInTabsOf(const Window* window) const
{
    if(!window)
        return false;

    for(const Window* host { getTabHostWindow() }; host; host = host->getTabHostWindow())
    {
        if(host == window)
            return true;
    }

    return false;
}

void RetroFuturaGUI::Window::resetShownTabs()
{
    //Each WindowTab marks itself again when it interacts. One in an inactive scene or an unselected tab, which doesn't, takes no dropped windows
    for(WindowTab* tab : _windowTabs)
    {
        if(!tab)
            continue;

        tab->_isShown = false;

        for(uSize index = 0; index < tab->GetWindowCount(); ++index)
        {
            Window* window { tab->GetWindow(index) };

            if(!window)
                continue;

            window->resetShownTabs();
        }
    }
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