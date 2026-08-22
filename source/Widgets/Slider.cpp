#include "Slider.hpp"
#include "IWidget.hpp"
#include "Input.hpp"
#include "PlatformBridge.hpp"
#include "Rectangle.hpp"
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::Slider::Slider(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Slider;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _grabBackground = std::make_unique<Rectangle>(projection);
    _grabBorder = std::make_unique<Rectangle>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if (_grabBackground)
        _grabBackground->SetRectangleMode(RectangleMode::Plane);

    if (_grabBorder)
        _grabBorder->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::Slider::Connect_OnValueChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueChangedAsync.Connect(slot);
    else
        _onValueChanged.Connect(slot);
}

void RetroFuturaGUI::Slider::Connect_OnValueSet(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueSetAsync.Connect(slot);
    else
        _onValueSet.Connect(slot);
}

void RetroFuturaGUI::Slider::Disconnect_OnValueChanged(const typename Signal<>::Slot& slot)
{
    _onValueChanged.Disconnect(slot);
    _onValueChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Slider::Disconnect_OnValueSet(const typename Signal<>::Slot& slot)
{
    _onValueSet.Disconnect(slot);
    _onValueSetAsync.Disconnect(slot);
}

/*void RetroFuturaGUI::Slider::EnableSliderButtons(const bool value)
{
    _useSliderButtons = value;

    if(!_buttonLower && _useSliderButtons)
        _buttonLower = std::make_unique<Button>(_name + "/ButtonLower", &_projection, this, _widgetTypeID, _parentWindow);
    
    if(!_buttonHigher && _useSliderButtons)
        _buttonHigher = std::make_unique<Button>(_name + "/ButtonHigher", &_projection, this, _widgetTypeID, _parentWindow);
}*/

void  RetroFuturaGUI::Slider::Draw()
{
    interact();
    
    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

    if (_grabBackground)
        _grabBackground->Draw();

    if (_grabBorder)
        _grabBorder->Draw();

    if(!_useSliderButtons)
        return;

    /*if(_buttonLower)
        _buttonLower->Draw();

    if(_buttonHigher)
        _buttonHigher->Draw();*/
}

void RetroFuturaGUI::Slider::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if (_background)
        _background->SetPosition(position);

    if (_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    setGrabPosition();
}

void RetroFuturaGUI::Slider::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if (_background)
        _background->SetSize(size);

    if (_border)
        _border->SetSize(size);

    setGrabSize();
    setGrabPosition();
}

void RetroFuturaGUI::Slider::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if (_background)
        _background->SetRotation(rotation);

    if (_border)
        _border->SetRotation(rotation);

    setGrabPosition();
}

void RetroFuturaGUI::Slider::SetGrabSize(const f32 size, const ElementSizing sizingMode)
{
    _grabSizingFactor = size;
    _grabSizingMode = sizingMode;
    setGrabSize();
}

void RetroFuturaGUI::Slider::setGrabSize()
{
    if(!_background)
        return;

    glm::vec3 size(0.0f);

    if(_grabSizingMode == ElementSizing::Percent)
        size = glm::vec3(_background->GetSize().x * _grabSizingFactor * 0.01f, _background->GetSize().y, 0.01f);
    else
        size = glm::vec3(_grabSizingFactor, _background->GetSize().y, 0.01f);

    if(_grabBackground)
        _grabBackground->SetSize(size);

    if(_grabBorder)
        _grabBorder->SetSize(size);
}

void RetroFuturaGUI::Slider::setGrabPosition()
{
    if(!_background || !_grabBackground)
        return;

    calculateGrabSliderPosition();

    // The grab always travels along the background's local x-axis.
    // Rotate that local offset by the widget's rotation to place it correctly in world space.
    const glm::vec2 localOffset(_grabSliderPosition - _background->GetSize().x * 0.5f, 0.0f);
    const f32 radians = glm::radians(_rotation.z);
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    const glm::vec3 position
    (
        _background->GetPosition().x + rotatedOffset.x,
        _background->GetPosition().y + rotatedOffset.y,
        _background->GetPosition().z + 0.02f
    );

    _grabBackground->SetPosition(position);
    _grabBackground->SetRotation(_rotation);

    if(_grabBorder)
    {
        _grabBorder->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
        _grabBorder->SetRotation(_rotation);
    }
}

void RetroFuturaGUI::Slider::calculateGrabSliderPosition()
{
    f32
        value { 0.0f },
        minValue { 0.0f },
        maxValue { 1.0f };

    switch(_valueType)
    {
        case SliderValueType::Int8:
            value = static_cast<f32>(_value.Int8);
            minValue = static_cast<f32>(_minValue.Int8);
            maxValue = static_cast<f32>(_maxValue.Int8);
        break;
        case SliderValueType::Int16:
            value = static_cast<f32>(_value.Int16);
            minValue = static_cast<f32>(_minValue.Int16);
            maxValue = static_cast<f32>(_maxValue.Int16);
        break;
        case SliderValueType::Int32:
            value = static_cast<f32>(_value.Int32);
            minValue = static_cast<f32>(_minValue.Int32);
            maxValue = static_cast<f32>(_maxValue.Int32);
        break;
        case SliderValueType::Int64:
            value = static_cast<f32>(_value.Int64);
            minValue = static_cast<f32>(_minValue.Int64);
            maxValue = static_cast<f32>(_maxValue.Int64);
        break;
        case SliderValueType::UInt8:
            value = static_cast<f32>(_value.UInt8);
            minValue = static_cast<f32>(_minValue.UInt8);
            maxValue = static_cast<f32>(_maxValue.UInt8);
        break;
        case SliderValueType::UInt16:
            value = static_cast<f32>(_value.UInt16);
            minValue = static_cast<f32>(_minValue.UInt16);
            maxValue = static_cast<f32>(_maxValue.UInt16);
        break;
        case SliderValueType::UInt32:
            value = static_cast<f32>(_value.UInt32);
            minValue = static_cast<f32>(_minValue.UInt32);
            maxValue = static_cast<f32>(_maxValue.UInt32);
        break;
        case SliderValueType::UInt64:
            value = static_cast<f32>(_value.UInt64);
            minValue = static_cast<f32>(_minValue.UInt64);
            maxValue = static_cast<f32>(_maxValue.UInt64);
        break;
        case SliderValueType::Float32:
            value = _value.Float32;
            minValue = _minValue.Float32;
            maxValue = _maxValue.Float32;
        break;
        case SliderValueType::Float64:
            value = static_cast<f32>(_value.Float64);
            minValue = static_cast<f32>(_minValue.Float64);
            maxValue = static_cast<f32>(_maxValue.Float64);
        break;
        default: // Bool
            value = _value.Bool ? 1.0f : 0.0f;
            minValue = 0.0f;
            maxValue = 1.0f;
        break;
    }

    const f32
        range { maxValue - minValue },
        fraction { range != 0.0f ? (value - minValue) / range : 0.0f },
        trackLength { _background->GetSize().x },
        grabLength { _grabBackground->GetSize().x },
        travelRange { trackLength - grabLength > 0.0f ? trackLength - grabLength : 0.0f };

                                      //v consider grab's half length
    _grabSliderPosition = grabLength * 0.5f + fraction * travelRange;
}

void RetroFuturaGUI::Slider::setValueFromMousePosition(const glm::vec2& mousePos)
{
    if(!_background || !_grabBackground)
        return;

    // Revert the widget's rotation to bring the mouse position into the background's local space where the track always runs along the local X axis
    const glm::vec2 translated { mousePos - glm::vec2(_background->GetPosition().x, _background->GetPosition().y) };
    const f32 radians { glm::radians(-_rotation.z) };
    const glm::vec2 localMouse
    (
        translated.x * cos(radians) - translated.y * sin(radians),
        translated.x * sin(radians) + translated.y * cos(radians)
    );

    const f32
        trackLength { _background->GetSize().x },
        grabLength { _grabBackground->GetSize().x },
        travelRange { trackLength - grabLength > 0.0f ? trackLength - grabLength : 0.0f },
        trackNearEdge { -trackLength * 0.5f },
        mouseOffsetOnAxis { localMouse.x - trackNearEdge - grabLength * 0.5f },
        clampedOffset { mouseOffsetOnAxis < 0.0f ? 0.0f : (mouseOffsetOnAxis > travelRange ? travelRange : mouseOffsetOnAxis) },
        fraction { travelRange != 0.0f ? clampedOffset / travelRange : 0.0f };

    switch(_valueType)
    {
        case SliderValueType::Int8:
            _value.Int8 = static_cast<i8>(_minValue.Int8 + fraction * (_maxValue.Int8 - _minValue.Int8));
        break;
        case SliderValueType::Int16:
            _value.Int16 = static_cast<i16>(_minValue.Int16 + fraction * (_maxValue.Int16 - _minValue.Int16));
        break;
        case SliderValueType::Int32:
            _value.Int32 = static_cast<i32>(_minValue.Int32 + fraction * (_maxValue.Int32 - _minValue.Int32));
        break;
        case SliderValueType::Int64:
            _value.Int64 = static_cast<i64>(_minValue.Int64 + fraction * (_maxValue.Int64 - _minValue.Int64));
        break;
        case SliderValueType::UInt8:
            _value.UInt8 = static_cast<u8>(_minValue.UInt8 + fraction * (_maxValue.UInt8 - _minValue.UInt8));
        break;
        case SliderValueType::UInt16:
            _value.UInt16 = static_cast<u16>(_minValue.UInt16 + fraction * (_maxValue.UInt16 - _minValue.UInt16));
        break;
        case SliderValueType::UInt32:
            _value.UInt32 = static_cast<u32>(_minValue.UInt32 + fraction * (_maxValue.UInt32 - _minValue.UInt32));
        break;
        case SliderValueType::UInt64:
            _value.UInt64 = static_cast<u64>(_minValue.UInt64 + fraction * (_maxValue.UInt64 - _minValue.UInt64));
        break;
        case SliderValueType::Float32:
            _value.Float32 = _minValue.Float32 + fraction * (_maxValue.Float32 - _minValue.Float32);
        break;
        case SliderValueType::Float64:
            _value.Float64 = _minValue.Float64 + static_cast<f64>(fraction) * (_maxValue.Float64 - _minValue.Float64);
        break;
        default: // Bool
            _value.Bool = fraction >= 0.5f;
        break;
    }

    _onValueChanged.Emit();
    _onValueChangedAsync.EmitAsync();

    setGrabPosition();
}

void RetroFuturaGUI::Slider::SetGrabBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _grabBackgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _grabBackgroundColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _grabBackgroundColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _grabBackgroundColorEnabled.assign(colors.begin(), colors.end());
    }

    setGrabBackgroundColors();
}

void RetroFuturaGUI::Slider::SetGrabBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _grabBorderColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _grabBorderColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _grabBorderColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _grabBorderColorEnabled.assign(colors.begin(), colors.end());
    }

    setGrabBorderColors();
}

void RetroFuturaGUI::Slider::SetCornerRadii(const glm::vec4& radii)
{
    if(_background)
        _background->SetCornerRadii(radii);

    if(_border)
        _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::Slider::SetGrabCornerRadii(const glm::vec4& radii)
{
    if(_grabBackground)
        _grabBackground->SetCornerRadii(radii);

    if(_grabBorder)
        _grabBorder->SetCornerRadii(radii);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundFillType(const FillType fillType)
{
    if(_grabBackground)
        _grabBackground->SetFillType(fillType);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundGradientOffset(const f32 gradientOffset)
{
    if(_grabBackground)
        _grabBackground->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_grabBackground)
        _grabBackground->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundGradientDegree(const f32 degree)
{
    if(_grabBackground)
        _grabBackground->SetGradientDegree(degree);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_grabBackground)
        _grabBackground->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::Slider::SetGrabWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_grabBackground)
        _grabBackground->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundDotColor(const glm::vec4& color)
{
    if(_grabBackground)
        _grabBackground->SetDotColor(color);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundDotDistance(const f32 distance)
{
    if(_grabBackground)
        _grabBackground->SetDotDistance(distance);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundDotSizeTransferDegree(const f32 degree)
{
    if(_grabBackground)
        _grabBackground->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _grabBackgroundDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(_grabBackground)
        _grabBackground->SetDotRadiusTransfer(_grabBackgroundDotRadiusTransfer);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_grabBackground)
        _grabBackground->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundDotAnimationSpeed(const f32 animationSpeed)
{
    if(_grabBackground)
        _grabBackground->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundFogAlpha(const f32 alpha)
{
    if(_grabBackground)
        _grabBackground->SetFogAlpha(alpha);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundFogSpeed(const f32 speed)
{
    if(_grabBackground)
        _grabBackground->SetFogSpeed(speed);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundFogDensity(std::span<f32> density)
{
    _grabBackgroundFogDensity.assign(density.begin(), density.end());

    if(_grabBackground)
        _grabBackground->SetFogDensity(_grabBackgroundFogDensity);
}

void RetroFuturaGUI::Slider::SetGrabBackgroundFogClearing(const f32 clearing)
{
    if(_grabBackground)
        _grabBackground->SetFogClearing(clearing);
}

void RetroFuturaGUI::Slider::SetGrabBorderFillType(const FillType fillType)
{
    if(_grabBorder)
        _grabBorder->SetFillType(fillType);
}

void RetroFuturaGUI::Slider::SetGrabBorderGradientOffset(const f32 gradientOffset)
{
    if(_grabBorder)
        _grabBorder->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::Slider::SetGrabBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_grabBorder)
        _grabBorder->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Slider::SetGrabBorderGradientDegree(const f32 degree)
{
    if(_grabBorder)
        _grabBorder->SetGradientDegree(degree);
}

void RetroFuturaGUI::Slider::SetGrabBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_grabBorder)
        _grabBorder->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::Slider::SetGrabWindowBorderImageTextureID(const u32 textureID)
{
    if(_grabBorder)
        _grabBorder->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Slider::SetGrabBorderDotColor(const glm::vec4& color)
{
    if(_grabBorder)
        _grabBorder->SetDotColor(color);
}

void RetroFuturaGUI::Slider::SetGrabBorderDotDistance(const f32 distance)
{
    if(_grabBorder)
        _grabBorder->SetDotDistance(distance);
}

void RetroFuturaGUI::Slider::SetGrabBorderDotSizeTransferDegree(const f32 degree)
{
    if(_grabBorder)
        _grabBorder->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::Slider::SetGrabBorderDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _grabBorderDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(_grabBorder)
        _grabBorder->SetDotRadiusTransfer(_grabBorderDotRadiusTransfer);
}

void RetroFuturaGUI::Slider::SetGrabBorderDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_grabBorder)
        _grabBorder->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::Slider::SetGrabBorderDotAnimationSpeed(const f32 animationSpeed)
{
    if(_grabBorder)
        _grabBorder->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Slider::SetGrabBorderWidth(const f32 borderWidth)
{
    if(_grabBorder)
        _grabBorder->SetBorderWidth(borderWidth);
}

void RetroFuturaGUI::Slider::SetGrabBorderGaps(std::span<BorderGap> gaps)
{
    if(_grabBorder)
        _grabBorder->SetBorderGaps(gaps);
}

void RetroFuturaGUI::Slider::setGrabBackgroundColors()
{
    if(_grabBackground) 

    switch(_grabBackgroundColorState)
    {
        case ColorState::Enabled:
            _grabBackground->SetColors(_grabBackgroundColorEnabled);
        break;
        case ColorState::Clicked:
            _grabBackground->SetColors(_grabBackgroundColorClicked);
        break;
        case ColorState::Hover:
            _grabBackground->SetColors(_grabBackgroundColorHover);
        break;
        default: //Disabled
            _grabBackground->SetColors(_grabBackgroundColorDisabled);
    }
}

void RetroFuturaGUI::Slider::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
}

void RetroFuturaGUI::Slider::setGrabColors(const ColorState state)
{
    _grabBackgroundColorState = state;
    _grabBorderColorState = state;
    setGrabBackgroundColors();
    setGrabBorderColors();
}

void RetroFuturaGUI::Slider::setGrabBorderColors()
{
    if(_grabBorder)

    switch(_grabBorderColorState)
    {
        case ColorState::Enabled:
            _grabBorder->SetColors(_grabBorderColorEnabled);
        break;
        case ColorState::Clicked:
            _grabBorder->SetColors(_grabBorderColorClicked);
        break;
        case ColorState::Hover:
            _grabBorder->SetColors(_grabBorderColorHover);
        break;
        default: //Disabled
            _grabBorder->SetColors(_grabBorderColorDisabled);
    }
}

bool RetroFuturaGUI::Slider::isInsideGrab(const glm::vec2& mousePos)
{
    const glm::vec2 grabPosition(_grabBackground->GetPosition().x, _grabBackground->GetPosition().y);
    return isPointInsideRect(mousePos, glm::vec3(_grabBackground->GetSize(), 0.0f), _grabBackground->GetPosition(), _grabBackground->GetRotation());
}

void RetroFuturaGUI::Slider::interact()
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
    bool isMouseInsideGrab = isPointInsideRect(mousePos, glm::vec3(_grabBackground->GetSize(), 0.0f), _grabBackground->GetPosition(), _grabBackground->GetRotation());

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
            setGrabColors(ColorState::Enabled);
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

        if(isMouseInsideGrab)
            setGrabColors(ColorState::Hover);
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);

        if(isMouseInsideGrab)
        {
            setGrabColors(ColorState::Clicked);
            _isDraggingGrab = true;
        }
        else
        {
            setValueFromMousePosition(mousePos);   
            _onValueChanged.Emit();
            _onValueChangedAsync.EmitAsync();
        }

        _previousGrabPosition = mousePos;
    }
    else if(isMouseButtonPressed && _wasClicked) //held (drag)
    {
        if(_isDraggingGrab // consider grab might be rotated
            && (_previousGrabPosition.x != mousePos.x || _previousGrabPosition.y != mousePos.y))
        {
            setValueFromMousePosition(mousePos);
            _previousGrabPosition = mousePos;
            _onValueChanged.Emit();
            _onValueChangedAsync.EmitAsync();
        }
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();
        _isDraggingGrab = false;

        if(isHovering)
        {
            setColors(ColorState::Hover);

            if(isMouseInsideGrab)
                setGrabColors(ColorState::Hover);
        }
        else
        {
            setColors(ColorState::Enabled);

            if(isMouseInsideGrab)
                setGrabColors(ColorState::Enabled);
        }
    }

    _wasClicked = isMouseButtonPressed;
}