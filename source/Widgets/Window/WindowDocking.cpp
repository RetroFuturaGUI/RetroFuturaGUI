#include "Window.hpp"
#include "WindowTab.hpp"
#include <algorithm>
#include <cmath>

bool RetroFuturaGUI::Window::DockWindow(Window* window, const DockEdge edge)
{
    return dockWindowAt(window, edge, dockedWindows(edge).size());
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
    getDockRoot()->layoutDockedWindows();
}

void RetroFuturaGUI::Window::EnableSquaredCornersWhenDocked(const bool enable)
{
    _noCornerRadiiWhenDocked = enable;
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

RetroFuturaGUI::Window* RetroFuturaGUI::Window::getDockRoot()
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

    const DockRect host { getScreenRect() };
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

    Window* root { getDockRoot() };

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

RetroFuturaGUI::Window::DockRect RetroFuturaGUI::Window::getScreenRect() const
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
    const DockRect rect { getScreenRect() };
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

glm::bvec4 RetroFuturaGUI::Window::getSquaredCorners() const
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
