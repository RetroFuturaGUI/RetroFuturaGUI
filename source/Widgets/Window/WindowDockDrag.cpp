#include "Window.hpp"
#include "WindowTab.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

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

    if(std::ranges::find(getAllWindows(), snap._Target) == getAllWindows().end())
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
        const glm::ivec4 content { _dockSnap._Tab->getContentScreenRect() };
        _dockSnap._Tab->showDropPreview(_windowTitle);
        showDockPreview(DockRect 
                            { ._Position = glm::ivec2(content.x, content.y), ._Size = glm::ivec2(content.z, content.w) },
                                _dockSnap._Tab->GetContentCornerRadii()
                        );
        return;
    }

    //The slot this window would take, with the windows already on that edge making room for it
    const DockRect host { _dockSnap._Target->getScreenRect() };
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

    const DockRect dragged { getScreenRect() };

    //A click on the title is no drag: the window has to have moved
    if(dragged._Position == _dragStartPosition)
        return {};

    const DockSnap tabSnap { findTabSnap() }; //Over a WindowTab's strip, dropping into the tab wins over docking to an edge!

    if(tabSnap._Tab)
        return tabSnap;

    DockSnap snap {};
    i32 closest { _snapDistance + 1 };

    for(Window* target : getAllWindows())
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

        const DockRect host { target->getScreenRect() };

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

    const glm::ivec2 cursor { getScreenRect()._Position + glm::ivec2(cursorX, cursorY) };

    for(Window* target : getAllWindows())
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

        const DockRect docked { window->getScreenRect() };

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

    const DockRect current { _dockPreview->getScreenRect() };

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
