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
    _indicator = std::make_unique<Rectangle>(projection);
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _parentGroup = parentGroup;

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::RadioButton::Draw()
{
    interact();
    drawBackground();
    drawBorder();

    if(!_isChecked)
        return;

    if(_indicator)
        _indicator->Draw();
}

void RetroFuturaGUI::RadioButton::SetEnabled(const bool enable, const bool emitSignal)
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

void RetroFuturaGUI::RadioButton::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    SetIndicatorPadding(_indicatorPadding);
}

void RetroFuturaGUI::RadioButton::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    if(_indicator)
        _indicator->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));
}

void RetroFuturaGUI::RadioButton::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_indicator)
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

void RetroFuturaGUI::RadioButton::SetIndicatorColors(std::span<glm::vec4> colors, const ColorState state)
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

    _background->SetCornerRadii(radii);

    if(!_border)
        return;

    _border->SetCornerRadii(radii);

    if(_indicator)
        _indicator->SetCornerRadii(radii - _border->GetBorderWidth() - _indicatorPadding);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientOffset(const f32 gradientOffset)
{
    if(_indicator)
        _indicator->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_indicator)
        _indicator->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientDegree(const f32 degree)
{
    if(_indicator)
        _indicator->SetGradientDegree(degree);
}

void RetroFuturaGUI::RadioButton::SetIndicatorGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_indicator)
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
    _indicatorPadding = pixels;
    glm::vec3 size { _size - glm::vec3(_border->GetBorderWidth() * 2, _border->GetBorderWidth() * 2, 0.0f) };
    size -= glm::vec3(_indicatorPadding * 2,  _indicatorPadding * 2, 0.0f);

    if(_indicator)
        _indicator->SetSize(size);
}

void RetroFuturaGUI::RadioButton::SetParentGroup(RadioButtonGroup* parentGroup)
{
    _parentGroup = parentGroup;
}