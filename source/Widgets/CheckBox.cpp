#include "CheckBox.hpp"
#include "IncludeHelper.hpp"
#include "ResourceManager.hpp"
#include "PlatformBridge.hpp"
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::CheckBox::CheckBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::CheckBox;
    _checkmark = ResourceManager::GetCheckmarkIcon();
    _checkmark->SetProjection(projection);
    _inherietFill = std::make_unique<Rectangle>(projection);
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::CheckBox::Draw()
{
    interact();
    drawBackground();
    drawBorder();

    if(_useInherietValue && _inherietValue != nullptr)
    {
        if(_inherietFill)
            _inherietFill->Draw();
    }

    if(!_isChecked)
        return;

    if(_checkmark)
        _checkmark->Draw();
}

void RetroFuturaGUI::CheckBox::SetEnabled(const bool enable, const bool emitSignal)
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

void RetroFuturaGUI::CheckBox::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    SetInnerPadding(_innerPadding);
}

void RetroFuturaGUI::CheckBox::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    if(_inherietFill)
        _inherietFill->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));

    if(_checkmark)
        _checkmark->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.03f));
}

void RetroFuturaGUI::CheckBox::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_inherietFill)
        _inherietFill->SetRotation(rotation);

    if(_checkmark)
        _checkmark->SetRotation(rotation);
}

void RetroFuturaGUI::CheckBox::SetValue(const bool value, const bool emitSignal)
{
    bool valueCahnged { false };

    if(_useInherietValue)
    {
        if(!_inherietValue)
            return;

        valueCahnged = *_inherietValue != value;
        *_inherietValue = value;
    }
    else
    {
        valueCahnged = _isChecked != value;
        _isChecked = value;
    }

    if(!emitSignal)
        return;

    _onSetValueAsync.EmitAsync();
    _onSetValue.Emit();

    if(!valueCahnged)
        return;

    _onValueChanged.Emit();
    _onValueChangedAsync.Emit();
}

void RetroFuturaGUI::CheckBox::SetInheritValueReference(bool* ref)
{
    _inherietValue = ref;
}

bool RetroFuturaGUI::CheckBox::GetValue() const
{
    if(_useInherietValue)
    {
        if(_inherietValue == nullptr)
            return false;
        else
            return *_inherietValue;
    }

    return _isChecked;
}

void RetroFuturaGUI::CheckBox::SetCheckmarkColors(std::span<glm::vec4> colors, const ColorState state)
{
    _checkmarkColorState = state;

    switch(state)
    {
        case ColorState::Enabled:
            _checkmarkColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _checkmarkColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _checkmarkColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _checkmarkColorsDisabled.assign(colors.begin(), colors.end());
    }

    setCheckmarkColors();
}
        
void RetroFuturaGUI::CheckBox::SetCornerRadii(const glm::vec4& radii)
{
    if(_background)
        _background->SetCornerRadii(radii);

    if(_border)
        _border->SetCornerRadii(radii);

    if(_inherietFill)
        _inherietFill->SetCornerRadii(radii);
}

void RetroFuturaGUI::CheckBox::SetInherietColors(std::span<glm::vec4> colors, const ColorState state)
{
    _inherietFillColorState = state;

    switch(state)
    {
        case ColorState::Enabled:
            _inherietFillColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _inherietFillColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _inherietFillColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _inherietFillColorsDisabled.assign(colors.begin(), colors.end());
    }

    setInherietFillColors();
}

void RetroFuturaGUI::CheckBox::SetInherietGradientOffset(const f32 gradientOffset)
{
    if(_inherietFill)
        _inherietFill->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::CheckBox::SetInherietGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_inherietFill)
        _inherietFill->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::CheckBox::SetInherietGradientDegree(const f32 degree)
{
    if(_inherietFill)
        _inherietFill->SetGradientDegree(degree);
}

void RetroFuturaGUI::CheckBox::SetInherietGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_inherietFill)
        _inherietFill->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::CheckBox::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    _checkmarkColorState = state;
    _inherietFillColorState = state;
    setBackgroundColors();
    setBorderColors();
    setCheckmarkColors();
    setInherietFillColors();
}

void RetroFuturaGUI::CheckBox::setCheckmarkColors()
{
    if(!_checkmark)
        return;

    switch(_checkmarkColorState)
    {
        case ColorState::Enabled:
            _checkmarkPathFill.colors = _checkmarkColorsEnabled;
        break;
        case ColorState::Clicked:
            _checkmarkPathFill.colors = _checkmarkColorsClicked;
        break;
        case ColorState::Hover:
            _checkmarkPathFill.colors = _checkmarkColorsHover;
        break;
        default: //Disabled
            _checkmarkPathFill.colors = _checkmarkColorsDisabled;
    }
            
    _checkmark->SetPathFill("CheckmarkPath", _checkmarkPathFill);
}

void RetroFuturaGUI::CheckBox::setInherietFillColors()
{
    if(!_inherietFill)
        return;

    switch(_inherietFillColorState)
    {
        case ColorState::Enabled:
            _inherietFill->SetColors(_inherietFillColorsEnabled);
        break;
        case ColorState::Clicked:
            _inherietFill->SetColors(_inherietFillColorsClicked);
        break;
        case ColorState::Hover:
            _inherietFill->SetColors(_inherietFillColorsHover);
        break;
        default: //Disabled
            _inherietFill->SetColors(_inherietFillColorsDisabled);
    }
}

void RetroFuturaGUI::CheckBox::interact()
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
    bool isMouseInside = hasMousePosition && isPointInside(mousePos);

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
        _isChecked = !_isChecked;
        _onValueChangedAsync.EmitAsync();
        _onValueChanged.Emit();
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

void RetroFuturaGUI::CheckBox::UseInherietedValue(const bool useInheriet)
{
    _useInherietValue = useInheriet;
}

void RetroFuturaGUI::CheckBox::SetInnerPadding(const f32 pixels)
{
    _innerPadding = pixels;
    glm::vec3 size { _size - glm::vec3(_border->GetBorderWidth() * 2.0f, _border->GetBorderWidth() * 2.0f, 0.0f) };

    if(_inherietFill)
        _inherietFill->SetSize(size);

    size -= glm::vec3( _innerPadding * 2.0f,  _innerPadding * 2.0f, 0.0f);

    if(_checkmark)
        _checkmark->SetSize(size);
}