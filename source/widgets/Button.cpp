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
        setColors(ColorSetKeys::Enabled);
        return;
    }

    _onDisableAsync.EmitAsync();
    _onDisable.Emit();
    setColors(ColorSetKeys::Disabled);
}

bool RetroFuturaGUI::Button::IsEnabled() const
{
    return _isEnabledFlag;
}

void RetroFuturaGUI::Button::SetBackgroundColor(const glm::vec4 &color)
{
    _rectangle->SetColor(color);
}

glm::vec4 RetroFuturaGUI::Button::GetBackgroundColor() const
{
    return _rectangle->GetColor();
}

void RetroFuturaGUI::Button::SetTextColor(const glm::vec4 &color)
{
    _text->SetColor(color);
}

glm::vec4 RetroFuturaGUI::Button::GetTextColor() const
{
    return _text->GetColor();
}

void RetroFuturaGUI::Button::SetLineBorderColor(const glm::vec4 &color)
{
    _lineBorder->SetColor(color);
}

glm::vec4 RetroFuturaGUI::Button::GetLineBorderColor() const
{
    return _lineBorder->GetColor();
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
            setColors(ColorSetKeys::Enabled);
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
        setColors(ColorSetKeys::Hover);
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorSetKeys::Clicked);
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            setColors(ColorSetKeys::Hover);
        else
            setColors(ColorSetKeys::Enabled);
    }

    _wasClicked = isMouseButtonPressed;
}

void RetroFuturaGUI::Button::setColors(const ColorSetKeys key)
{
    switch(key)
    {
        case ColorSetKeys::Enabled:
        {
            SetBackgroundColor(_backgroundColorEnabled);
            SetLineBorderColor(_borderColorEnabled);
            SetTextColor(_textColorEnabled);
        } break;
        case ColorSetKeys::Clicked:
        {
            SetBackgroundColor(_backgroundColorClicked);
            SetLineBorderColor(_borderColorClicked);
            SetTextColor(_textColorClicked);
        } break;
        case ColorSetKeys::Hover:
        {
            SetBackgroundColor(_backgroundColorHover);
            SetLineBorderColor(_borderColorHover);
            SetTextColor(_textColorHover);
        } break;
        default: //Disabled
        {
            SetBackgroundColor(_backgroundColorDisabled);
            SetLineBorderColor(_borderColorDisabled);
            SetTextColor(_textColorDisabled);
        }
    }
}
