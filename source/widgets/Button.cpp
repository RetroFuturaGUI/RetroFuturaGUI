#include "Button.hpp"

RetroFuturaGUI::Button::Button(const IdentityParams &identity, const GeometryParams2D &geometry, const TextParams &textParams, const BorderParams& borderParams)
    : IWidget(identity, geometry)
{
    _rectangle = std::make_unique<Rectangle>(geometry, _backgroundColorEnabled);
    _lineBorder = std::make_unique<LineBorder>(geometry, borderParams);
    _text = std::make_unique<Text>(geometry, textParams);
}

void RetroFuturaGUI::Button::Draw()
{
    interact();
    _rectangle->Draw();
    _lineBorder->Draw();
    _text->Draw();
}

void RetroFuturaGUI::Button::Connect_OnClick(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onClick.Connect(slot);
    else
        _onClickAsync.Connect(slot); 
}

void RetroFuturaGUI::Button::Connect_OnRelease(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onRelease.Connect(slot);
    else
        _onReleaseAsync.Connect(slot); 
}

void RetroFuturaGUI::Button::Connect_WhileHover(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _whileHover.Connect(slot);
    else
        _whileHoverAsync.Connect(slot); 
}

void RetroFuturaGUI::Button::Connect_OnMouseEnter(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onMouseEnter.Connect(slot);
    else
        _onMouseEnterAsync.Connect(slot); 
}

void RetroFuturaGUI::Button::Connect_OnMouseLeave(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onMouseLeave.Connect(slot);
    else
        _onMouseLeaveAsync.Connect(slot); 
}

void RetroFuturaGUI::Button::Connect_OnEnable(const typename Signal<>::Slot & slot, const bool async)
{
    if (async)
        _onEnable.Connect(slot);
    else
        _onEnableAsync.Connect(slot); 
}

void RetroFuturaGUI::Button::Connect_OnDisable(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onDisable.Connect(slot);
    else
        _onDisableAsync.Connect(slot); 
}

void RetroFuturaGUI::Button::Disconnect_OnClick(const typename Signal<>::Slot &slot)
{
    _onClick.Disconnect(slot);
    _onClickAsync.Disconnect(slot);
}

void RetroFuturaGUI::Button::Disconnect_OnRelease(const typename Signal<>::Slot &slot)
{
    _onRelease.Disconnect(slot);
    _onReleaseAsync.Disconnect(slot);
}

void RetroFuturaGUI::Button::Disconnect_WhileHover(const typename Signal<>::Slot &slot)
{
    _whileHover.Disconnect(slot);
    _whileHoverAsync.Disconnect(slot);
}

void RetroFuturaGUI::Button::Disconnect_OnMouseEnter(const typename Signal<>::Slot &slot)
{
    _onMouseEnter.Disconnect(slot);
    _onMouseEnterAsync.Disconnect(slot);
}

void RetroFuturaGUI::Button::Disconnect_OnMouseLeave(const typename Signal<>::Slot &slot)
{
    _onMouseLeave.Disconnect(slot);
    _onMouseLeaveAsync.Disconnect(slot);
}

void RetroFuturaGUI::Button::Disconnect_OnEnable(const typename Signal<>::Slot &slot)
{
    _onEnable.Disconnect(slot);
    _onEnableAsync.Disconnect(slot);
}

void RetroFuturaGUI::Button::Disconnect_OnDisable(const typename Signal<>::Slot &slot)
{
    _onDisable.Disconnect(slot);
    _onDisableAsync.Disconnect(slot);
}

void RetroFuturaGUI::Button::SetEnabled(const bool enable)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        _onEnableAsync.EmitAsync();
        _onEnable.Emit();
        _state = ColorSetState::Enabled;
        setColors();
        return;
    }

    _onDisableAsync.EmitAsync();
    _onDisable.Emit();
    ColorSetState::Disabled;
    setColors();
}

bool RetroFuturaGUI::Button::IsEnabled() const
{
    return _isEnabledFlag;
}

void RetroFuturaGUI::Button::SetBackgroundColor(const glm::vec4& color, const ColorSetState state)
{
    switch(state)
    {
        case ColorSetState::Clicked:
            _backgroundColorClicked = color;
        break;
        case ColorSetState::Disabled:
            _backgroundColorDisabled = color;
        break;
        case ColorSetState::Hover:
            _backgroundColorHover = color;
        break;
        default: // Enabled
            _backgroundColorEnabled = color;
    }

    setColors();
}

glm::vec4 RetroFuturaGUI::Button::GetBackgroundColor(const ColorSetState state) const
{
    switch(state)
    {
        case ColorSetState::Clicked:
            return _backgroundColorClicked;
        case ColorSetState::Disabled:
            return _backgroundColorDisabled;
        case ColorSetState::Hover:
            return _backgroundColorHover;
        default: // Enabled
            return _backgroundColorEnabled;
    }
}

void RetroFuturaGUI::Button::SetTextColor(const glm::vec4& color, const ColorSetState state)
{
    switch(state)
    {
        case ColorSetState::Clicked:
            _textColorClicked = color;
        break;
        case ColorSetState::Disabled:
            _textColorDisabled = color;
        break;
        case ColorSetState::Hover:
            _textColorHover = color;
        break;
        default: // Enabled
            _textColorEnabled = color;
    }

    setColors();
}

glm::vec4 RetroFuturaGUI::Button::GetTextColor(const ColorSetState state) const
{
    switch(state)
    {
        case ColorSetState::Clicked:
            return _textColorClicked;
        case ColorSetState::Disabled:
            return _textColorDisabled;
        case ColorSetState::Hover:
            return _textColorHover;
        default: // Enabled
            return _textColorEnabled;
    }
}

void RetroFuturaGUI::Button::SetLineBorderColor(const glm::vec4& color, const ColorSetState state)
{
    switch(state)
    {
        case ColorSetState::Clicked:
            _borderColorClicked = color;
        break;
        case ColorSetState::Disabled:
            _borderColorDisabled = color;
        break;
        case ColorSetState::Hover:
            _borderColorHover = color;
        break;
        default: // Enabled
            _borderColorEnabled = color;
    }

    setColors();
}

glm::vec4 RetroFuturaGUI::Button::GetLineBorderColor(const ColorSetState state) const
{
    switch(state)
    {
        case ColorSetState::Clicked:
            return _borderColorClicked;
        case ColorSetState::Disabled:
            return _borderColorDisabled;
        case ColorSetState::Hover:
            return _borderColorHover;
        default: // Enabled
            return _borderColorEnabled;
    }
}

void RetroFuturaGUI::Button::interact()
{
    auto mousePos = InputManager::GetMousePosition();
    bool isMouseButtonPressed = InputManager::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool isMouseInside = isPointInside(glm::vec2(mousePos));

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            _state = ColorSetState::Enabled;
            setColors();
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
        _state = ColorSetState::Hover;
        setColors();
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        _state = ColorSetState::Clicked;
        setColors();
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            _state = ColorSetState::Hover;
        else
            _state = ColorSetState::Enabled;

        setColors();
    }

    _wasClicked = isMouseButtonPressed;
}

void RetroFuturaGUI::Button::setColors()
{
    switch(_state)
    {
        case ColorSetState::Enabled:
        {
            setBackgroundColorElement(_backgroundColorEnabled);
            setLineBorderColorElement(_borderColorEnabled);
            setTextColorElement(_textColorEnabled);
        } break;
        case ColorSetState::Clicked:
        {
            setBackgroundColorElement(_backgroundColorClicked);
            setLineBorderColorElement(_borderColorClicked);
            setTextColorElement(_textColorClicked);
        } break;
        case ColorSetState::Hover:
        {
            setBackgroundColorElement(_backgroundColorHover);
            setLineBorderColorElement(_borderColorHover);
            setTextColorElement(_textColorHover);
        } break;
        default: //Disabled
        {
            setBackgroundColorElement(_backgroundColorDisabled);
            setLineBorderColorElement(_borderColorDisabled);
            setTextColorElement(_textColorDisabled);
        }
    }
}

void RetroFuturaGUI::Button::setBackgroundColorElement(const glm::vec4 &color)
{
    _rectangle->SetColor(color);
}

void RetroFuturaGUI::Button::setTextColorElement(const glm::vec4 &color)
{
    _text->SetColor(color);
}

void RetroFuturaGUI::Button::setLineBorderColorElement(const glm::vec4 &color)
{
    _lineBorder->SetColor(color);
}
