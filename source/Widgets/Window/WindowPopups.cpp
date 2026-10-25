#include "Window.hpp"
#include "PlatformBridge.hpp"
#include <utility>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

bool RetroFuturaGUI::Window::PushPopup(std::unique_ptr<IPopup> popup)
{
    if(!popup)
        return false;

    //The window holding the tab takes the popups, so they block the tab strip too
    if(IsInTab())
    {
        Window* host { getTabHostWindow() };

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
    Window* owner { getTopPopupWindow() };
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
        const Window* host { getTabHostWindow() };

        if(!host)
            return false;

        return host->IsBlockedByPopup();
    }

    if(!_popups.empty())
        return true;

    if(!_popupHost)
        return false;

    //A popup's window is blocked unless it is the top of the stack
    return _popupHost->getTopPopupWindow() != this;
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

    Window* topWindow { getTopPopupWindow() };

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
    getTopPopupWindow()->_ignoreHeldPress = true; //The window below takes the mouse again, but not the press on the button that closed the popup

    if(!closing)
        return;

    closing->emitClosed();
    closing.reset();

    //Destroying the popup's window left no context current
    glfwMakeContextCurrent(_window);
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::getTopPopupWindow()
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
        over { window->getScreenRect() },
        own { getScreenRect() };
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
