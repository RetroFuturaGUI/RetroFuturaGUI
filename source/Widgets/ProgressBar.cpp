#include "ProgressBar.hpp"
#include "Input.hpp"
#include "PlatformBridge.hpp"
#include "Rectangle.hpp"
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ProgressBar::ProgressBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IRangedValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ProgressBar;
}

void  RetroFuturaGUI::ProgressBar::Draw()
{
    interact();

    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

    drawGraph();
    drawIndicator();
}

void RetroFuturaGUI::ProgressBar::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
}

bool RetroFuturaGUI::ProgressBar::isInsideGraph(const glm::vec2& mousePos)
{
    return isPointInsideRect(mousePos, glm::vec3(_graph->GetSize(), 0.0f), _graph->GetPosition(), _graph->GetRotation());
}

void RetroFuturaGUI::ProgressBar::interact()
{
    i32
        mouseX { 0 },
        mouseY { 0 };
    bool hasMousePosition { false };

#if defined(TARGET_PLATFORM_LINUX)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_parentWindow), mouseX, mouseY);
#elif defined(TARGET_PLATFORM_WINDOWS)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_parentWindow), mouseX, mouseY);
#endif

    glm::vec2 mousePos { static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY) };
    bool isMouseButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left);
    bool isMouseInside = hasMousePosition && isPointInsideRect(mousePos, _size, _position, _rotation);
    bool isMouseInsideGraph = isPointInsideRect(mousePos, glm::vec3(_graph->GetSize(), 0.0f), _graph->GetPosition(), _graph->GetRotation());

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
            setGraphColors(ColorState::Enabled);
        }

        return;
    }

    bool isHovering = _isEnabledFlag && isMouseInside;

    if(isHovering) // hover
    {
        _whileHoverAsync.EmitAsync();
        _whileHover.Emit();
    }

    if (isHovering && !_mouseLeftFlag && !_mouseEnteredFlag) //enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
        setColors(ColorState::Hover);

        if(isMouseInsideGraph)
            setGraphColors(ColorState::Hover);
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
        setGraphColors(ColorState::Clicked);
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
        {
            setColors(ColorState::Hover);

            if(isMouseInsideGraph)
                setGraphColors(ColorState::Hover);
        }
        else
        {
            setColors(ColorState::Enabled);

            if(isMouseInsideGraph)
                setGraphColors(ColorState::Enabled);
        }
    }

    _wasClicked = isMouseButtonPressed;
}