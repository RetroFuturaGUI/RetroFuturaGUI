#include "Button.hpp"

RetroFuturaGUI::Button::Button(const IdentityParams& identity, const GeometryParams3D& geometry, const TextParams& textParams, const float borderWidth)
    : IWidget(identity, geometry)
{
    _widgetTypeID = WidgetTypeID::Button;
    _rectangle = std::make_unique<Rectangle>(geometry, _backgroundColorEnabled, RectangleMode::PLANE);
    _border = std::make_unique<Rectangle>(geometry, _borderColorEnabled, RectangleMode::BORDER);
    
    if(_border) 
        _border->SetBorderWidth(borderWidth);

    if(textParams._Text.size() > 0)
        _text = std::make_unique<Text>(geometry, textParams);
}

void RetroFuturaGUI::Button::Draw()
{
    interact();
    drawBackgroundBorder();
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

    if(_rectangle)
        _rectangle->SetSize(size);

    if(_border)
        _border->SetSize(size);

    //if(_text)    
        //_text->Resize(size); //add extra text resizing logic
}

void RetroFuturaGUI::Button::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_rectangle)
        _rectangle->SetPosition(position);

    if(_border)
        _border->SetPosition(position);

    if(_text)
        _text->SetPosition(position);
}

void RetroFuturaGUI::Button::SetCornerRadii(const glm::vec4& radii)
{
    if(_rectangle)
        _rectangle->SetCornerRadii(radii);

    if(_border)    
        _border->SetCornerRadii(radii);
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

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            setColors(ColorState::Hover);
        else
            setColors(ColorState::Enabled);
    }

    _wasClicked = isMouseButtonPressed;
}

void RetroFuturaGUI::Button::setColors(const ColorState state)
{
    _borderBackgroundColorState = state;
    _textColorState = state;
    setborderBackgroundColors();
    setTextColors();
}