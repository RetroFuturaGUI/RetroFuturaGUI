#include "RadioButton.hpp"
#include "RadioButtonGroup.hpp"
#include "IncludeHelper.hpp"
#include "PlatformBridge.hpp"
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::RadioButton::RadioButton(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, RadioButtonGroup* parentGroup)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::RadioButton;
    _indicator = std::make_unique<Plane>(projection);
    _background = std::make_unique<Plane>(projection);
    _border = std::make_unique<Frame>(projection);
    _parentGroup = parentGroup;

    if(!_background)
        return;

    if(!_border)
        return;

}

void RetroFuturaGUI::RadioButton::Draw()
{
    if(!_indicator)
        return;

    drawBackground();
    drawBorder();

    if(!_isChecked)
        return;

    _indicator->Draw();
}

void RetroFuturaGUI::RadioButton::SetSize(const glm::vec3& size)
{
    if(!_background)
        return;

    if(!_border)
        return;

    IWidget::SetSize(size);

    _background->SetSize(size);
    _border->SetSize(size);

    SetIndicatorPadding(_indicatorPadding);
}

void RetroFuturaGUI::RadioButton::SetPosition(const glm::vec3& position)
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_indicator)
        return;

    IWidget::SetPosition(position);

    _background->SetPosition(position);
    _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
    _indicator->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));
}

void RetroFuturaGUI::RadioButton::SetRotation(const glm::vec3& rotation)
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_indicator)
        return;

    _rotation = rotation;

    _background->SetRotation(rotation);
    _border->SetRotation(rotation);
    _indicator->SetRotation(rotation);
}

void RetroFuturaGUI::RadioButton::SetValue(const bool value, const bool emitSignal)
{
    bool valueCahnged { _isChecked != value };
    _isChecked = value;
    
    if(_isChecked && _parentGroup)
        _parentGroup->DeselectOthers(this);

    if(!emitSignal)
        return;

    _onSetValueAsync.EmitAsync();
    _onSetValue.Emit();

    if(!valueCahnged)
        return;

    _onValueChanged.Emit();
    _onValueChangedAsync.Emit();
}

bool RetroFuturaGUI::RadioButton::GetValue() const
{
    return _isChecked;
}

void RetroFuturaGUI::RadioButton::SetIndicatorColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _indicatorColorState = state;

    switch(state)
    {
        case ColorState::Enabled:
            _indicatorColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _indicatorColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _indicatorColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _indicatorColorsDisabled.assign(colors.begin(), colors.end());
    }

    setIndicatorColors();
}
        
void RetroFuturaGUI::RadioButton::SetCornerRadii(const glm::vec4& radii)
{
    if(!_background)
        return;

    if(!_indicator)
        return;

    _background->SetCornerRadii(radii);

    if(!_border)
        return;

    _border->SetCornerRadii(radii);

    _indicator->SetCornerRadii(radii - _border->GetBorderWidth() - _indicatorPadding);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientOffset(const f32 gradientOffset)
{
    if(!_indicator)
        return;

    _indicator->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_indicator)
        return;

    _indicator->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientDegree(const f32 degree)
{
    if(!_indicator)
        return;

    _indicator->SetGradientDegree(degree);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_indicator)
        return;

    _indicator->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::RadioButton::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    _indicatorColorState = state;
    setBackgroundColors();
    setBorderColors();
    setIndicatorColors();
}

void RetroFuturaGUI::RadioButton::setIndicatorColors()
{
    if(!_indicator)
        return;

    switch(_indicatorColorState)
    {
        case ColorState::Enabled:
            _indicator->SetColors(_indicatorColorsEnabled);
        break;
        case ColorState::Clicked:
            _indicator->SetColors(_indicatorColorsClicked);
        break;
        case ColorState::Hover:
            _indicator->SetColors(_indicatorColorsHover);
        break;
        default: //Disabled
            _indicator->SetColors(_indicatorColorsDisabled);
    }
}

void RetroFuturaGUI::RadioButton::interact()
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

        if(!_isChecked)
            SetValue(!_isChecked, true);

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

void RetroFuturaGUI::RadioButton::SetIndicatorPadding(const f32 pixels)
{
    if(!_indicator)
        return;

    _indicatorPadding = pixels;
    glm::vec3 size { _size - glm::vec3(_border->GetBorderWidth() * 2, _border->GetBorderWidth() * 2, 0.0f) };
    size -= glm::vec3(_indicatorPadding * 2,  _indicatorPadding * 2, 0.0f);

    _indicator->SetSize(size);
}

void RetroFuturaGUI::RadioButton::SetParentGroup(RadioButtonGroup* parentGroup)
{
    _parentGroup = parentGroup;
}