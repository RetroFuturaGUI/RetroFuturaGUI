#include "Button.hpp"

RetroFuturaGUI::Button::Button(const IdentityParams& identity, const GeometryParams2D& geometry, const TextParams& textParams, const float borderWidth)
    : IWidget(identity, geometry)
{
    _widgetTypeID = WidgetTypeID::Button;
    _rectangle = std::make_unique<Rectangle>(geometry, _backgroundColorEnabled, RectangleMode::PLANE);
    _border = std::make_unique<Rectangle>(geometry, _borderColorEnabled, RectangleMode::BORDER);
    
    if(_border) 
        _border->SetBorderWidth(borderWidth);

    _text = std::make_unique<Text>(geometry, textParams);
}

void RetroFuturaGUI::Button::Draw()
{
    interact();

    if(_rectangle)
        _rectangle->Draw();

    if(_border)
        _border->Draw();

    if(_text)
        _text->Draw();
}

void RetroFuturaGUI::Button::Connect_OnClick(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onClickAsync.Connect(slot);
    else
        _onClick.Connect(slot); 
}

void RetroFuturaGUI::Button::Connect_OnRelease(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onReleaseAsync.Connect(slot);
    else 
        _onRelease.Connect(slot);
}

void RetroFuturaGUI::Button::Connect_WhileHover(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _whileHoverAsync.Connect(slot);
    else
        _whileHover.Connect(slot);
}

void RetroFuturaGUI::Button::Connect_OnMouseEnter(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onMouseEnterAsync.Connect(slot);
    else
        _onMouseEnter.Connect(slot);
}

void RetroFuturaGUI::Button::Connect_OnMouseLeave(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onMouseLeaveAsync.Connect(slot);
    else
        _onMouseLeave.Connect(slot);
}

void RetroFuturaGUI::Button::Connect_OnEnable(const typename Signal<>::Slot & slot, const bool async)
{
    if (async)
        _onEnableAsync.Connect(slot);
    else
        _onEnable.Connect(slot);
}

void RetroFuturaGUI::Button::Connect_OnDisable(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onDisableAsync.Connect(slot);
    else
        _onDisable.Connect(slot);
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
            _backgroundColorClicked.clear();
            _backgroundColorClicked.resize(1, color);
        break;
        case ColorSetState::Disabled:
            _backgroundColorDisabled.clear();
            _backgroundColorDisabled.resize(1, color);
        break;
        case ColorSetState::Hover:
            _backgroundColorHover.clear();
            _backgroundColorHover.resize(1, color);
        break;
        default: // Enabled
            _backgroundColorEnabled.clear();
            _backgroundColorEnabled.resize(1, color);
    }

    setColors();
}

void RetroFuturaGUI::Button::SetBackgroundColors(std::span<glm::vec4> colors, const ColorSetState state)
{
    switch(state)
    {
        case ColorSetState::Clicked:
            _backgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorSetState::Disabled:
            _backgroundColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorSetState::Hover:
            _backgroundColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _backgroundColorEnabled.assign(colors.begin(), colors.end());
    }

    setColors();
}

void RetroFuturaGUI::Button::SetBorderColor(const glm::vec4 & color, const ColorSetState state)
{
        switch(state)
    {
        case ColorSetState::Clicked:
            _borderColorClicked.clear();
            _borderColorClicked.resize(1, color);
        break;
        case ColorSetState::Disabled:
            _borderColorDisabled.clear();
            _borderColorDisabled.resize(1, color);
        break;
        case ColorSetState::Hover:
            _borderColorClicked.clear();
            _borderColorClicked.resize(1, color);
        break;
        default: // Enabled
            _borderColorEnabled.clear();
            _borderColorEnabled.resize(1, color);
    }

    setColors();
}

void RetroFuturaGUI::Button::SetBorderColors(std::span<glm::vec4> colors, const ColorSetState state)
{
        switch(state)
    {
        case ColorSetState::Clicked:
            _borderColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorSetState::Disabled:
            _borderColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorSetState::Hover:
            _borderColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _borderColorEnabled.assign(colors.begin(), colors.end());
    }

    setColors();
}

std::vector<glm::vec4> RetroFuturaGUI::Button::GetBackgroundColors(const ColorSetState state) const
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
            _textColorClicked.resize(1, color);
        break;
        case ColorSetState::Disabled:
            _textColorDisabled.resize(1, color);
        break;
        case ColorSetState::Hover:
            _textColorHover.resize(1, color);
        break;
        default: // Enabled
            _textColorEnabled.resize(1, color);
    }

    setColors();
}

std::vector<glm::vec4> RetroFuturaGUI::Button::GetTextColor(const ColorSetState state) const
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
            _borderColorClicked.resize(1, color);
        break;
        case ColorSetState::Disabled:
            _borderColorDisabled.resize(1, color);
        break;
        case ColorSetState::Hover:
            _borderColorHover.resize(1, color);
        break;
        default: // Enabled
            _borderColorEnabled.resize(1, color);
    }

    setColors();
}

std::vector<glm::vec4> RetroFuturaGUI::Button::GetLineBorderColor(const ColorSetState state) const
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

void RetroFuturaGUI::Button::SetSize(const glm::vec2 &size)
{
    IWidget::SetSize(size);

    if(_rectangle)
        _rectangle->SetSize(size);

    if(_border)
        _border->SetSize(size);

    //if(_text)    
        //_text->Resize(size); //add extra text resizing logic
}

void RetroFuturaGUI::Button::SetPosition(const glm::vec2 &position)
{
    IWidget::SetPosition(position);

    if(_rectangle)
        _rectangle->SetPosition(position);

    if(_border)
        _border->SetPosition(position);

    if(_text)
        _text->SetPosition(position);
}

void RetroFuturaGUI::Button::SetCornerRadii(const glm::vec4 & radii)
{
    if(_rectangle)
        _rectangle->SetCornerRadii(radii);

    if(_border)    
        _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::Button::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_rectangle)
        _rectangle->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Button::SetBorderGradientOffset(const f32 gradientOffset)
{
    if(_border) 
        _border->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::Button::SetBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_border) 
        _border->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Button::SetBorderGradientDegree(const f32 degree)
{
    if(_border) 
        _border->SetGradientDegree(degree);
}

void RetroFuturaGUI::Button::SetBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_border) 
        _border->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::Button::SetRotation(const float rotation)
{
    _rotation = rotation;

    if(_rectangle)
        _rectangle->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);
}

void RetroFuturaGUI::Button::interact()
{
    auto mousePos = InputManager::GetMousePositionInvertedY();
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
            setBorderColorElement(_borderColorEnabled);
            setTextColorElement(_textColorEnabled.front());
        } break;
        case ColorSetState::Clicked:
        {
            setBackgroundColorElement(_backgroundColorClicked);
            setBorderColorElement(_borderColorClicked);
            setTextColorElement(_textColorClicked.front());
        } break;
        case ColorSetState::Hover:
        {
            setBackgroundColorElement(_backgroundColorHover);
            setBorderColorElement(_borderColorHover);
            setTextColorElement(_textColorHover.front());
        } break;
        default: //Disabled
        {
            setBackgroundColorElement(_backgroundColorDisabled);
            setBorderColorElement(_borderColorDisabled);
            setTextColorElement(_textColorDisabled.front());
        }
    }
}

void RetroFuturaGUI::Button::setBackgroundColorElement(std::vector<glm::vec4>& color)
{
    if(_rectangle) 
        _rectangle->SetColor(color);
}

void RetroFuturaGUI::Button::setTextColorElement(const glm::vec4& color)
{
    if(_text) 
        _text->SetColor(color);
}

void RetroFuturaGUI::Button::setBorderColorElement(std::vector<glm::vec4>& color)
{
    if(_border) 
        _border->SetColor(color);
}
