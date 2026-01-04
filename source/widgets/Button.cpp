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
        _state = ColorState::Enabled;
        setColors();
        return;
    }

    _onDisableAsync.EmitAsync();
    _onDisable.Emit();
    ColorState::Disabled;
    setColors();
}

bool RetroFuturaGUI::Button::IsEnabled() const
{
    return _isEnabledFlag;
}

void RetroFuturaGUI::Button::SetBackgroundColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _backgroundColorClicked.clear();
            _backgroundColorClicked.resize(1, color);
        break;
        case ColorState::Disabled:
            _backgroundColorDisabled.clear();
            _backgroundColorDisabled.resize(1, color);
        break;
        case ColorState::Hover:
            _backgroundColorHover.clear();
            _backgroundColorHover.resize(1, color);
        break;
        default: // Enabled
            _backgroundColorEnabled.clear();
            _backgroundColorEnabled.resize(1, color);
    }

    setColors();
}

void RetroFuturaGUI::Button::SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _backgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _backgroundColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _backgroundColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _backgroundColorEnabled.assign(colors.begin(), colors.end());
    }

    setColors();
}

void RetroFuturaGUI::Button::SetBackgroundGradientOffset(const f32 gradientOffset)
{
    if(_rectangle)
        _rectangle->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::Button::SetBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_rectangle)
        _rectangle->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Button::SetBackgroundGradientDegree(const f32 degree)
{
    if(_rectangle)
        _rectangle->SetGradientDegree(degree);
}

void RetroFuturaGUI::Button::SetBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{    
    if(_rectangle)
        _rectangle->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::Button::SetBorderColor(const glm::vec4 & color, const ColorState state)
{
        switch(state)
    {
        case ColorState::Clicked:
            _borderColorClicked.clear();
            _borderColorClicked.resize(1, color);
        break;
        case ColorState::Disabled:
            _borderColorDisabled.clear();
            _borderColorDisabled.resize(1, color);
        break;
        case ColorState::Hover:
            _borderColorClicked.clear();
            _borderColorClicked.resize(1, color);
        break;
        default: // Enabled
            _borderColorEnabled.clear();
            _borderColorEnabled.resize(1, color);
    }

    setColors();
}

void RetroFuturaGUI::Button::SetBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
        switch(state)
    {
        case ColorState::Clicked:
            _borderColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _borderColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _borderColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _borderColorEnabled.assign(colors.begin(), colors.end());
    }

    setColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::Button::GetBackgroundColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _backgroundColorClicked;
        case ColorState::Disabled:
            return _backgroundColorDisabled;
        case ColorState::Hover:
            return _backgroundColorHover;
        default: // Enabled
            return _backgroundColorEnabled;
    }
}

void RetroFuturaGUI::Button::SetTextColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _textColorClicked.resize(1, color);
        break;
        case ColorState::Disabled:
            _textColorDisabled.resize(1, color);
        break;
        case ColorState::Hover:
            _textColorHover.resize(1, color);
        break;
        default: // Enabled
            _textColorEnabled.resize(1, color);
    }

    setColors();
}

std::vector<glm::vec4> RetroFuturaGUI::Button::GetTextColor(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _textColorClicked;
        case ColorState::Disabled:
            return _textColorDisabled;
        case ColorState::Hover:
            return _textColorHover;
        default: // Enabled
            return _textColorEnabled;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::Button::GetBorderColor(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _borderColorClicked;
        case ColorState::Disabled:
            return _borderColorDisabled;
        case ColorState::Hover:
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

void RetroFuturaGUI::Button::SetWindowBorderImageTextureID(const u32 textureID)
{
    if(_border)
        _border->SetWindowBackgroundImageTextureID(textureID);
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
            _state = ColorState::Enabled;
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
        _state = ColorState::Hover;
        setColors();
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        _state = ColorState::Clicked;
        setColors();
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            _state = ColorState::Hover;
        else
            _state = ColorState::Enabled;

        setColors();
    }

    _wasClicked = isMouseButtonPressed;
}

void RetroFuturaGUI::Button::setColors()
{
    switch(_state)
    {
        case ColorState::Enabled:
        {
            setBackgroundColorElement(_backgroundColorEnabled);
            setBorderColorElement(_borderColorEnabled);
            setTextColorElement(_textColorEnabled.front());
        } break;
        case ColorState::Clicked:
        {
            setBackgroundColorElement(_backgroundColorClicked);
            setBorderColorElement(_borderColorClicked);
            setTextColorElement(_textColorClicked.front());
        } break;
        case ColorState::Hover:
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
