#include "Button.hpp"
#include "Parts/Border.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::Button::Button(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Button;
    _text.SetTransitionSettings(&_transitionSettings);
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);
}

void RetroFuturaGUI::Button::Draw()
{
    _background.Draw();
    _border.Draw();
    _text.Draw();
}

void RetroFuturaGUI::Button::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);

    _background.SetSize(size);
    _border.SetSize(size);

    _text.SetSize(glm::vec2(size.x, size.y));
    _text.SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.02f));
}

void RetroFuturaGUI::Button::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);

    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
    _text.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));
}

void RetroFuturaGUI::Button::interact()
{
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    glm::vec2 mousePos { pointer._Position };
    bool 
        isMouseButtonPressed { pointer._LeftButtonPressed },
        isMouseInside { pointer._HasPosition && isPointInside(mousePos) };
    
    const bool //drop presses that started outside and were released inside this widget. A release outside ends the press without triggering OnRelease
        isPressStart { isMouseButtonPressed && !_wasClicked },
        isRelease { !isMouseButtonPressed && _wasClicked };
    _wasClicked = isMouseButtonPressed;

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(isRelease)
            _pressStartedInside = false;

        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
        setColors(ColorState::Enabled);
        }

        return;
    }
    
    bool isHovering = _isEnabledFlag && isMouseInside;
    if(isHovering) // hover
    {
        requestCursorIcon(PlatformBridge::CursorIcon::Hand);
        _whileHoverAsync.EmitAsync();
        _whileHover.Emit();
    }

    if (isHovering && !_mouseLeftFlag && !_mouseEnteredFlag) //enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
        setColors(ColorState::Hover);
    }

    if(isPressStart) //click
    {
        _pressStartedInside = true;
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
    }
    else if(isRelease && _pressStartedInside) //release
    {
        _pressStartedInside = false;
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            setColors(ColorState::Hover);
        else
            setColors(ColorState::Enabled);
    }
}

void RetroFuturaGUI::Button::setColors(const ColorState state)
{
    _background.SetColorState(state);
    _border.SetColorState(state);
    _text.SetColorState(state);
}

RetroFuturaGUI::Background& RetroFuturaGUI::Button::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::Button::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::Button::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::Button::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::Button::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::Button::GetText() const
{
    return _text;
}