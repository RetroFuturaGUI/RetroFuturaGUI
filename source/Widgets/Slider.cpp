#include "Slider.hpp"
#include "IWidget.hpp"
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

RetroFuturaGUI::Slider::Slider(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IRangedValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Slider;
}

void  RetroFuturaGUI::Slider::Draw()
{
    interact();

    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

    drawIndicator();
    drawGraph();
}

void RetroFuturaGUI::Slider::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
}

bool RetroFuturaGUI::Slider::isInsideIndicator(const glm::vec2& mousePos)
{
    return isPointInsideRect(mousePos, glm::vec3(_indicatorBackground->GetSize(), 0.0f), _indicatorBackground->GetPosition(), _indicatorBackground->GetRotation());
}

void RetroFuturaGUI::Slider::interact()
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
    bool isMouseInsideIndicator = isPointInsideRect(mousePos, glm::vec3(_indicatorBackground->GetSize(), 0.0f), _indicatorBackground->GetPosition(), _indicatorBackground->GetRotation());

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
            setIndicatorColors(ColorState::Enabled);
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

        if(isMouseInsideIndicator)
            setIndicatorColors(ColorState::Hover);
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);

        if(isMouseInsideIndicator)
        {
            setIndicatorColors(ColorState::Clicked);
            _isDraggingIndicator = true;
        }
        else
        {
            //Every click inside the track that misses the indicator now jumps the value - the buttons
            //that used to be excluded here live outside the slider, in their own cells
            setValueFromMousePosition(mousePos);
            _onValueChanged.Emit();
            _onValueChangedAsync.EmitAsync();
        }

        _previousIndicatorPosition = mousePos;
    }
    else if(isMouseButtonPressed && _wasClicked) //held (drag)
    {
        if(_isDraggingIndicator // consider indicator might be rotated
            && (_previousIndicatorPosition.x != mousePos.x || _previousIndicatorPosition.y != mousePos.y))
        {
            setValueFromMousePosition(mousePos);
            _previousIndicatorPosition = mousePos;
            _onValueChanged.Emit();
            _onValueChangedAsync.EmitAsync();
        }
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();
        _isDraggingIndicator = false;

        if(isHovering)
        {
            setColors(ColorState::Hover);

            if(isMouseInsideIndicator)
                setIndicatorColors(ColorState::Hover);
        }
        else
        {
            setColors(ColorState::Enabled);

            if(isMouseInsideIndicator)
                setIndicatorColors(ColorState::Enabled);
        }
    }

    _wasClicked = isMouseButtonPressed;
}