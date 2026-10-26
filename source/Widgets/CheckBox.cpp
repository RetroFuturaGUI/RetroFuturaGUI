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
    _inheritFill = std::make_unique<Plane>(projection);
    _background = std::make_unique<Plane>(projection);
    _border = std::make_unique<Frame>(projection);

    if(!_background)
        return;

    if(!_border)
        return;

}

void RetroFuturaGUI::CheckBox::Draw()
{
    if(!_inheritFill)
        return;

    interact();
    drawBackground();
    drawBorder();

    if(_useInheritValue && _inheritValue)
    {
        _inheritFill->Draw();
    }

    if(!_isChecked)
        return;

    if(!_checkmark)
        return;

    //Every CheckBox and Table in every window shares the checkmark, so it takes this box's projection and place right before drawing
    _checkmark->SetProjection(&_projection);
    _checkmark->SetSize(_checkmarkSize);
    _checkmark->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.03f));
    _checkmark->SetRotation(_rotation);
    _checkmark->Draw();
}

void RetroFuturaGUI::CheckBox::SetSize(const glm::vec3& size)
{
    if(!_background)
        return;

    if(!_border)
        return;

    IWidget::SetSize(size);

    _background->SetSize(size);
    _border->SetSize(size);

    SetInnerPadding(_innerPadding);
}

void RetroFuturaGUI::CheckBox::SetPosition(const glm::vec3& position)
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_inheritFill)
        return;

    IWidget::SetPosition(position);

    _background->SetPosition(position);
    _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
    _inheritFill->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));
}

void RetroFuturaGUI::CheckBox::SetRotation(const glm::vec3& rotation)
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_inheritFill)
        return;

    _rotation = rotation;

    _background->SetRotation(rotation);
    _border->SetRotation(rotation);
    _inheritFill->SetRotation(rotation);
}

void RetroFuturaGUI::CheckBox::SetValue(const bool value, const bool emitSignal)
{
    bool valueCahnged { false };

    if(_useInheritValue)
    {
        if(!_inheritValue)
            return;

        valueCahnged = *_inheritValue != value;
        *_inheritValue = value;
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
    _inheritValue = ref;
}

bool RetroFuturaGUI::CheckBox::GetValue() const
{
    if(_useInheritValue)
    {
        if(!_inheritValue)
            return false;
        else
            return *_inheritValue;
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
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_inheritFill)
        return;

    _background->SetCornerRadii(radii);
    _border->SetCornerRadii(radii);
    _inheritFill->SetCornerRadii(radii);
}

void RetroFuturaGUI::CheckBox::SetInheritColors(std::span<glm::vec4> colors, const ColorState state)
{
    _inheritFillColorState = state;

    switch(state)
    {
        case ColorState::Enabled:
            _inheritFillColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _inheritFillColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _inheritFillColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _inheritFillColorsDisabled.assign(colors.begin(), colors.end());
    }

    setInheritFillColors();
}

void RetroFuturaGUI::CheckBox::SetInheritGradientOffset(const f32 gradientOffset)
{
    if(!_inheritFill)
        return;

    _inheritFill->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::CheckBox::SetInheritGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_inheritFill)
        return;

    _inheritFill->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::CheckBox::SetInheritGradientDegree(const f32 degree)
{
    if(!_inheritFill)
        return;

    _inheritFill->SetGradientDegree(degree);
}

void RetroFuturaGUI::CheckBox::SetInheritGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_inheritFill)
        return;

    _inheritFill->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::CheckBox::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    _checkmarkColorState = state;
    _inheritFillColorState = state;
    setBackgroundColors();
    setBorderColors();
    setCheckmarkColors();
    setInheritFillColors();
}

void RetroFuturaGUI::CheckBox::setCheckmarkColors()
{
    if(!_checkmark)
        return;

    switch(_checkmarkColorState)
    {
        case ColorState::Enabled:
            _checkmarkPathFill._Colors = _checkmarkColorsEnabled;
        break;
        case ColorState::Clicked:
            _checkmarkPathFill._Colors = _checkmarkColorsClicked;
        break;
        case ColorState::Hover:
            _checkmarkPathFill._Colors = _checkmarkColorsHover;
        break;
        default: //Disabled
            _checkmarkPathFill._Colors = _checkmarkColorsDisabled;
    }
            
    _checkmark->SetPathFill("CheckmarkPath", _checkmarkPathFill);
}

void RetroFuturaGUI::CheckBox::setInheritFillColors()
{
    if(!_inheritFill)
        return;

    switch(_inheritFillColorState)
    {
        case ColorState::Enabled:
            _inheritFill->SetColors(_inheritFillColorsEnabled);
        break;
        case ColorState::Clicked:
            _inheritFill->SetColors(_inheritFillColorsClicked);
        break;
        case ColorState::Hover:
            _inheritFill->SetColors(_inheritFillColorsHover);
        break;
        default: //Disabled
            _inheritFill->SetColors(_inheritFillColorsDisabled);
    }
}

void RetroFuturaGUI::CheckBox::interact()
{
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    glm::vec2 mousePos { pointer._Position };
    bool
        isMouseButtonPressed { pointer._LeftButtonPressed },
        isMouseInside { pointer._HasPosition && isPointInside(mousePos) };

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

void RetroFuturaGUI::CheckBox::EnableInheritedValue(const bool enable)
{
    _useInheritValue = enable;
}

void RetroFuturaGUI::CheckBox::SetInnerPadding(const f32 pixels)
{
    if(!_inheritFill)
        return;

    _innerPadding = pixels;
    glm::vec3 size { _size - glm::vec3(_border->GetBorderWidth() * 2.0f, _border->GetBorderWidth() * 2.0f, 0.0f) };

    _inheritFill->SetSize(size);

    size -= glm::vec3( _innerPadding * 2.0f,  _innerPadding * 2.0f, 0.0f);
    _checkmarkSize = size;
}