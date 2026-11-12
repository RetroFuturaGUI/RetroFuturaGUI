#include "IPopup.hpp"
#include "Window.hpp"
#include "IDropDown.hpp"
#include "PlatformBridge.hpp"
#include "Scene.hpp"

RetroFuturaGUI::IPopup::IPopup(std::string_view name, const i32 width, const i32 height)
    : IPopup(name, width, height, StyleScope::Popup)
{
}

RetroFuturaGUI::IPopup::IPopup(std::string_view name, const i32 width, const i32 height, const StyleScope scope)
    : _openerContext(glfwGetCurrentContext())
{
    _window = std::unique_ptr<Window>(new Window(name, width, height, Window::WindowRole::Popup));
    _window->setStyleScope(scope); //before the popup makes its widgets, which take it from the window
}

RetroFuturaGUI::IPopup::~IPopup()
{
    if(!_window)
        return;

    GLFWwindow* glfwWindow { _window->GetGlfwWindow() };

    if(!glfwWindow)
        return;

    glfwMakeContextCurrent(glfwWindow);

    if(_isOnStack)
        return; //the window holding the stack makes its own context current once this has gone

    //Never opened (a refused Open() for instance) so nothing else would destroying the window which would leave no context current,
    //and the one current when the popup was made comes back
    _scenes.clear();
    _window.reset();
    glfwMakeContextCurrent(_openerContext);
}

RetroFuturaGUI::Window* RetroFuturaGUI::IPopup::GetWindow() const
{
    return _window.get();
}

void RetroFuturaGUI::IPopup::AddScene(std::unique_ptr<ISceneHost> host)
{
    if(!host)
        return;

    if(!_window)
        return;

    Scene* scene { host->GetScene() };

    if(!scene)
        return;

    _window->AddScene(scene);
    _scenes.push_back(std::move(host));
}

bool RetroFuturaGUI::IPopup::requestClose()
{
    if(!_window)
        return false;

    GLFWwindow* glfwWindow { _window->GetGlfwWindow() };

    if(!glfwWindow)
        return false;

    if(_window->IsBlockedByPopup())
        return false;

    _isCloseRequested = true;
    glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    return true;
}

bool RetroFuturaGUI::IPopup::allowsSystemClose()
{
    return true;
}

bool RetroFuturaGUI::IPopup::dismissesOnEscape() const
{
    return true;
}

void RetroFuturaGUI::IPopup::takeKeys()
{
    _escapePressCountSeen = PlatformBridge::Input::GetKeyPressCount(PB_KEY_ESCAPE); //an Escape that closed the popup above is spent.

    if(!_window)
        return;

    _window->TakeKeyboardInput();
}

void RetroFuturaGUI::IPopup::followKeys()
{
    const u32 escapePressCount { PlatformBridge::Input::GetKeyPressCount(PB_KEY_ESCAPE) };

    if(escapePressCount == _escapePressCountSeen)
        return;

    _escapePressCountSeen = escapePressCount;

    if(!dismissesOnEscape())
        return;

    if(!_window)
        return;

    //An open list closes first, as it would on a click elsewhere; the next Escape dismisses the popup.
    if(IDropDown::CloseOpenLists(_window->GetGlfwWindow()))
        return;

    Dismiss();
}

void RetroFuturaGUI::IPopup::restoreOpenerContext() const
{
    //restore context to the state when this window was opening
    glfwMakeContextCurrent(_openerContext);
}