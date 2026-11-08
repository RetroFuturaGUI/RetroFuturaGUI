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

RetroFuturaGUI::CheckBox::CheckBox(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::CheckBox;
    _inheritFill = std::make_unique<Plane>(projection);
    useTransitions({ _inheritFill.get() });
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);
}

void RetroFuturaGUI::CheckBox::Draw()
{
    if(!_inheritFill)
        return;

    _background.Draw();
    _border.Draw();

    if(_useInheritValue && _inheritValue)
        _inheritFill->Draw();

    if(!_isChecked)
        return;

    _checkmark.Draw();
}

void RetroFuturaGUI::CheckBox::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    _background.SetSize(size);
    _border.SetSize(size);
    SetInnerPadding(_innerPadding);
}

void RetroFuturaGUI::CheckBox::applyPosition(const glm::vec3& position)
{
    if(!_inheritFill)
        return;

    IWidget::applyPosition(position);
    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
    _inheritFill->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));
    _checkmark.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.03f));
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

void RetroFuturaGUI::CheckBox::updateInheritFillShape()
{
    if(!_inheritFill)
        return;

    _inheritFill->SetCornerRadii(_background.GetCornerRadii());
    _inheritFill->SetCutCorners(_background.GetCutCorners());
}

void RetroFuturaGUI::CheckBox::SetInheritColors(std::span<const glm::vec4> colors, const ColorState state)
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
    _checkmark.SetColorState(state);
    _inheritFillColorState = state;
    _background.SetColorState(state);
    _border.SetColorState(state);
    setInheritFillColors();
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
    glm::vec3 size { _size - glm::vec3(_border.GetWidth() * 2.0f, _border.GetWidth() * 2.0f, 0.0f) };

    _inheritFill->SetSize(size);
    size -= glm::vec3( _innerPadding * 2.0f,  _innerPadding * 2.0f, 0.0f);
    _checkmark.SetSize(size);
}

RetroFuturaGUI::Icon& RetroFuturaGUI::CheckBox::GetCheckmark()
{
    return _checkmark;
}

const RetroFuturaGUI::Icon& RetroFuturaGUI::CheckBox::GetCheckmark() const
{
    return _checkmark;
}

RetroFuturaGUI::Background& RetroFuturaGUI::CheckBox::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::CheckBox::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::CheckBox::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::CheckBox::GetBorder() const
{
    return _border;
}