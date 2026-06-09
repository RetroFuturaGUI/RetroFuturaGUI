#include "TextBox.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::TextBox::TextBox(const IdentityParams& identity, const GeometryParams3D& geometry, const TextParams& textParams, const float borderWidth)
    : IWidget(identity, geometry)
{
    _widgetTypeID = WidgetTypeID::TextBox;
    _rectangle = std::make_unique<Rectangle>(geometry, _backgroundColorEnabled, RectangleMode::PLANE);
    _border = std::make_unique<Rectangle>(geometry, _borderColorEnabled, RectangleMode::BORDER);
    
    if(_border) 
        _border->SetBorderWidth(borderWidth);

    if(textParams._Text.size() > 0)
        _text = std::make_unique<Text>(geometry, textParams);

    _prevKeyStates.resize(GLFW_KEY_LAST + 1, 0);
}

void RetroFuturaGUI::TextBox::Draw()
{
    interact();

    if(_rectangle)
        _rectangle->Draw();

    if(_border)
        _border->Draw();

    if(_text)
        _text->Draw();
}

void RetroFuturaGUI::TextBox::Connect_OnTextChange(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onTextChangeAsync.Connect(slot);
    else
        _onTextChange.Connect(slot);
}

void RetroFuturaGUI::TextBox::Disconnect_OnTextChange(const typename Signal<>::Slot &slot)
{
    _onTextChange.Disconnect(slot);
    _onTextChangeAsync.Disconnect(slot);
}

void RetroFuturaGUI::TextBox::SetEnabled(const bool enable, const bool emitSignal)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        if(emitSignal)
        {
            _onEnableAsync.EmitAsync();
            _onEnable.Emit();
        }

        _colorState = ColorState::Enabled;
        setColors();
        return;
    }

    if(emitSignal)
    {
        _onDisableAsync.EmitAsync();
        _onDisable.Emit();
    }

    _colorState = ColorState::Disabled;
    setColors();
}

void RetroFuturaGUI::TextBox::SetBackgroundColor(const glm::vec4& color, const ColorState state)
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

void RetroFuturaGUI::TextBox::SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
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

void RetroFuturaGUI::TextBox::SetBackgroundGradientOffset(const f32 gradientOffset)
{
    if(_rectangle)
        _rectangle->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::TextBox::SetBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_rectangle)
        _rectangle->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::TextBox::SetBackgroundGradientDegree(const f32 degree)
{
    if(_rectangle)
        _rectangle->SetGradientDegree(degree);
}

void RetroFuturaGUI::TextBox::SetBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{    
    if(_rectangle)
        _rectangle->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::TextBox::SetBorderColor(const glm::vec4 & color, const ColorState state)
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

void RetroFuturaGUI::TextBox::SetBorderColors(std::span<glm::vec4> colors, const ColorState state)
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

const std::vector<glm::vec4>& RetroFuturaGUI::TextBox::GetBackgroundColors(const ColorState state) const
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

void RetroFuturaGUI::TextBox::SetBackgroundFillType(const FillType fillType)
{
    _backgroundFillType = fillType;

    if(_rectangle)
        _rectangle->SetFillType(fillType);
}

void RetroFuturaGUI::TextBox::SetTextColor(const glm::vec4& color, const ColorState state)
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

std::vector<glm::vec4> RetroFuturaGUI::TextBox::GetTextColor(const ColorState state) const
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

const std::vector<glm::vec4>& RetroFuturaGUI::TextBox::GetBorderColor(const ColorState state) const
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

void RetroFuturaGUI::TextBox::SetBorderFillType(const FillType fillType)
{
    _borderFillType = fillType;

    if(_border)
        _border->SetFillType(fillType);
}

void RetroFuturaGUI::TextBox::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_rectangle)
        _rectangle->SetSize(size);

    if(_border)
        _border->SetSize(size);

    //if(_text)    
        //_text->Resize(size); //add extra text resizing logic
}

void RetroFuturaGUI::TextBox::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_rectangle)
        _rectangle->SetPosition(position);

    if(_border)
        _border->SetPosition(position);

    if(_text)
        _text->SetPosition(position);
}

void RetroFuturaGUI::TextBox::SetCornerRadii(const glm::vec4& radii)
{
    if(_rectangle)
        _rectangle->SetCornerRadii(radii);

    if(_border)    
        _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::TextBox::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_rectangle)
        _rectangle->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::TextBox::SetBorderGradientOffset(const f32 gradientOffset)
{
    if(_border) 
        _border->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::TextBox::SetBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_border) 
        _border->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::TextBox::SetBorderGradientDegree(const f32 degree)
{
    if(_border) 
        _border->SetGradientDegree(degree);
}

void RetroFuturaGUI::TextBox::SetBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_border) 
        _border->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::TextBox::SetWindowBorderImageTextureID(const u32 textureID)
{
    if(_border)
        _border->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::TextBox::SetRotation(const float rotation)
{
    _rotation = rotation;

    if(_rectangle)
        _rectangle->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);
}

void RetroFuturaGUI::TextBox::interact()
{
    auto mousePos = InputManager::GetMousePositionInvertedY();
    bool isMouseTextBoxPressed = InputManager::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool isMouseInside = isPointInside(glm::vec2(mousePos));

    if(_editingEnabled && !_mouseEnteredFlag && isMouseTextBoxPressed)
    {
        _editingEnabled = false;
    }

    editText();

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            _colorState = ColorState::Enabled;
            setColors();
        }

        PlatformBridge::Keyboard::SetActiveWindow(0);
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
        _colorState = ColorState::Hover;
        setColors();
    }

    if (isMouseTextBoxPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        _colorState = ColorState::Clicked;
        setColors();
        _editingEnabled = true;
#if defined(TARGET_PLATFORM_LINUX)
        PlatformBridge::Keyboard::SetActiveWindow(glfwGetX11Window(_parentWindow));
#elif defined(TARGET_PLATFORM_WINDOWS)
        PlatformBridge::Keyboard::SetActiveWindow(glfwGetWin32Window(_parentWindow));
#endif
    }
    else if(!isMouseTextBoxPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            _colorState = ColorState::Hover;
        else
            _colorState = ColorState::Enabled;

        setColors();
    }

    _wasClicked = isMouseTextBoxPressed;
}

void RetroFuturaGUI::TextBox::setColors()
{
    switch(_colorState)
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

void RetroFuturaGUI::TextBox::setBackgroundColorElement(std::vector<glm::vec4>& color)
{
    if(_rectangle) 
        _rectangle->SetColor(color);
}

void RetroFuturaGUI::TextBox::setTextColorElement(const glm::vec4& color)
{
    if(_text) 
        _text->SetColor(color);
}

void RetroFuturaGUI::TextBox::setBorderColorElement(std::vector<glm::vec4>& color)
{
    if(_border) 
        _border->SetColor(color);
}

void RetroFuturaGUI::TextBox::SetText(std::string_view text)
{
    if(_text)
        _text->SetText(text);

    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}
void RetroFuturaGUI::TextBox::SetReadOnly(const bool readOnly)
{
    _readOnly = readOnly;
}

bool RetroFuturaGUI::TextBox::IsReadOnly() const
{
    return _readOnly;
}

void RetroFuturaGUI::TextBox::editText()
{
    if(!_editingEnabled || !_text)
        return;

    if(_parentWindow != InputManager::GetFocusedWindow())
        return;

    static constexpr int keyRepeatInitialDelay = 60;
    static constexpr int keyRepeatInterval = 5;

    if(PlatformBridge::Keyboard::GetKeyboardUseState() == PlatformBridge::KeyboardUseState::KeyReleased)
    {
        _keyWasReleased = true;
        _keyHoldFrames = 0;
        _keyRepeatText.clear();
        return;
    }

    auto emitChange = [this]() 
    {
        _onTextChangeAsync.EmitAsync();
        _onTextChange.Emit();
    };

    if(!_keyWasReleased)
    {
        if (!_keyRepeatText.empty())
        {
            ++_keyHoldFrames;
            if (_keyHoldFrames >= keyRepeatInitialDelay && (_keyHoldFrames - keyRepeatInitialDelay) % keyRepeatInterval == 0)
            {
                _text->SetText(_text->GetText() + _keyRepeatText);
                emitChange();
            }
        }
        return;
    }

    if(PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_BACKSPACE) == PlatformBridge::KeyPressState::Press)
    {
        if(_text->GetText().size() == 0)
            return;

        uSize numDelete { 1 };
        
        for(uSize count { 0 }; ((u8)_text->GetText()[_text->GetText().size() - 1 - count] & (u8)0b11000000) == (u8)0b10000000; ++count)
            ++numDelete;
    
        _text->SetText(_text->GetText().substr(0, _text->GetText().size() - numDelete));
        _keyWasReleased = false;
        _keyHoldFrames = 0;
        _keyRepeatText.clear();
        emitChange();
        return;
    }

    const std::string keyText = PlatformBridge::Keyboard::GetInputString();
    if (!keyText.empty())
    {
        _text->SetText(_text->GetText() + keyText);
        _keyRepeatText = keyText;
        _keyWasReleased = false;
        _keyHoldFrames = 0;
        emitChange();
    }

    return;
}

const std::string& RetroFuturaGUI::TextBox::GetText() const
{
    return _text->GetText();
}