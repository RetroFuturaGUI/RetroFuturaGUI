#include "Button.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::Button::Button(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Button;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _text = std::make_unique<Text>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::Button::Draw()
{
    interact();
    drawBackground();
    drawBorder();
    drawText();
}

void RetroFuturaGUI::Button::SetEnabled(const bool enable, const bool emitSignal)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        if(emitSignal)
        {
            _onEnableAsync.EmitAsync();
            _onEnable.Emit();
        }

        setColors(ColorState::Enabled);
        return;
    }

    if(emitSignal)
    {
        _onDisableAsync.EmitAsync();
        _onDisable.Emit();
    }
        
    setColors(ColorState::Disabled);
}

void RetroFuturaGUI::Button::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    if(_text)
    {
        _text->SetParentSize(glm::vec2(size.x, size.y));
        _text->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.02f));
    }
}

void RetroFuturaGUI::Button::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    if(_text)
        _text->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));
}

void RetroFuturaGUI::Button::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);
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
    _backgroundColorState = state;
    _borderColorState = state;
    _textColorState = state;
    setBackgroundColors();
    setBorderColors();
    setTextColors();
}

void RetroFuturaGUI::Button::SetCornerRadii(const glm::vec4& radii)
{
    _background->SetCornerRadii(radii);
    _border->SetCornerRadii(radii);
}