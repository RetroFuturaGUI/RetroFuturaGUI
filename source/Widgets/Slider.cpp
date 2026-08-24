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
    _indicatorBackground = std::make_unique<Rectangle>(projection);
    _indicatorBorder = std::make_unique<Rectangle>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if (_indicatorBackground)
        _indicatorBackground->SetRectangleMode(RectangleMode::Plane);

    if (_indicatorBorder)
        _indicatorBorder->SetRectangleMode(RectangleMode::Border);

    _track = _background.get();
    _elementProjection = projection;
    _useIndicator = true;
}

void RetroFuturaGUI::Slider::EnableSliderButtons(const bool value)
{
    _useSliderButtons = value;

    if(_useSliderButtons)
    {
        if(!_buttonLower)
        {
            _buttonLower = std::make_unique<Button>(_name + "/ButtonLower", &_projection, this, _widgetTypeID, _parentWindow);
            _buttonLower->SetBackgroundColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), ColorState::Enabled);
            _buttonLower->SetBackgroundColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), ColorState::Hover);
            _buttonLower->SetBackgroundColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), ColorState::Clicked);
            _buttonLower->SetBackgroundColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), ColorState::Disabled);
            _buttonLower->SetBackgroundFillType(FillType::SOLID);
            _buttonLower->Connect_OnClick([this]() { stepValue(false); }, false);
        }

        if(!_buttonHigher)
        {
            _buttonHigher = std::make_unique<Button>(_name + "/ButtonHigher", &_projection, this, _widgetTypeID, _parentWindow);
            _buttonHigher->SetBackgroundColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), ColorState::Enabled);
            _buttonHigher->SetBackgroundColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), ColorState::Hover);
            _buttonHigher->SetBackgroundColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), ColorState::Clicked);
            _buttonHigher->SetBackgroundColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), ColorState::Disabled);
            _buttonHigher->SetBackgroundFillType(FillType::SOLID);
            _buttonHigher->Connect_OnClick([this]() { stepValue(true); }, false);
        }
    }

    SetSize(_size);
}

void  RetroFuturaGUI::Slider::Draw()
{
    interact();

    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

    drawIndicator();
    drawGraph();

    if(!_useSliderButtons)
        return;

    if(_buttonLower)
        _buttonLower->Draw();

    if(_buttonHigher)
        _buttonHigher->Draw();
}

void RetroFuturaGUI::Slider::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if (_background)
        _background->SetPosition(position);

    if (_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    setIndicatorPosition();
    setGraphPosition();
    setButtonPositions();
}

void RetroFuturaGUI::Slider::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    // Each button is a square with an edge length of _background's y size. Each button must be clamped within the Widget's space
    f32 trackWidth { size.x };

    if(_useSliderButtons)
        trackWidth = size.x - 2.0f * size.y > 0.0f ? size.x - 2.0f * size.y : 0.0f;

    const glm::vec3 trackSize(trackWidth, size.y, size.z);

    if (_background)
        _background->SetSize(trackSize);

    if (_border)
        _border->SetSize(trackSize);

    setIndicatorSize();
    setIndicatorPosition();
    setGraphPosition();
    setButtonSizes();
    setButtonPositions();
}

void RetroFuturaGUI::Slider::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if (_background)
        _background->SetRotation(rotation);

    if (_border)
        _border->SetRotation(rotation);

    setIndicatorPosition();
    setGraphPosition();
    setButtonPositions();
}

void RetroFuturaGUI::Slider::SetIndicatorSize(const f32 size, const ElementSizing sizingMode)
{
    _indicatorSizingFactor = size;
    _indicatorSizingMode = sizingMode;
    setIndicatorSize();
}

void RetroFuturaGUI::Slider::setIndicatorSize()
{
    if(!_background)
        return;

    glm::vec3 size(0.0f);

    if(_indicatorSizingMode == ElementSizing::Percent)
        size = glm::vec3(_background->GetSize().x * _indicatorSizingFactor * 0.01f, _background->GetSize().y, 0.01f);
    else
        size = glm::vec3(_indicatorSizingFactor, _background->GetSize().y, 0.01f);

    if(_indicatorBackground)
        _indicatorBackground->SetSize(size);

    if(_indicatorBorder)
        _indicatorBorder->SetSize(size);
}

void RetroFuturaGUI::Slider::setButtonSizes()
{
    if(!_background)
        return;

    const glm::vec3 size(_background->GetSize().y, _background->GetSize().y, 0.0f);

    if(_buttonLower)
        _buttonLower->SetSize(size);

    if(_buttonHigher)
        _buttonHigher->SetSize(size);
}

void RetroFuturaGUI::Slider::setButtonPositions()
{
    if(!_background)
        return;

    // The buttons flank the track along its local x-axis, just like the indicator; rotate that local offset into world space the same way setIndicatorPosition does.
    const f32
        buttonEdgeLength { _background->GetSize().y },
        trackHalfLength { _background->GetSize().x * 0.5f },
        radians { glm::radians(_rotation.z) };

    if(_buttonLower)
    {
        const f32 localOffset { -(trackHalfLength + buttonEdgeLength * 0.5f) };

        _buttonLower->SetPosition(glm::vec3
        (
            _background->GetPosition().x + localOffset * cos(radians),
            _background->GetPosition().y + localOffset * sin(radians),
            _background->GetPosition().z
        ));
        _buttonLower->SetRotation(_rotation);
    }

    if(_buttonHigher)
    {
        const f32 localOffset { trackHalfLength + buttonEdgeLength * 0.5f };

        _buttonHigher->SetPosition(glm::vec3
        (
            _background->GetPosition().x + localOffset * cos(radians),
            _background->GetPosition().y + localOffset * sin(radians),
            _background->GetPosition().z
        ));
        _buttonHigher->SetRotation(_rotation);
    }
}

void RetroFuturaGUI::Slider::stepValue(const bool increase)
{
    union
    {
        bool Bool;
        i8 Int8;
        i16 Int16;
        i32 Int32;
        i64 Int64;
        u8 UInt8;
        u16 UInt16;
        u32 UInt32;
        u64 UInt64;
        f32 Float32;
        f64 Float64;
    } clamped { .UInt64 = 0 };

    switch(_valueType)
    {
        case ValueType::Int8:
            clamped.Int8 = increase
                ? (_maxValue.Int8 - _value.Int8 < _buttonIncrement.Int8 ? _maxValue.Int8 : _value.Int8 + _buttonIncrement.Int8)
                : (_value.Int8 - _minValue.Int8 < _buttonIncrement.Int8 ? _minValue.Int8 : _value.Int8 - _buttonIncrement.Int8);
            SetValue<i8>(clamped.Int8);
        break;
        case ValueType::Int16:
            clamped.Int16 = increase
                ? (_maxValue.Int16 - _value.Int16 < _buttonIncrement.Int16 ? _maxValue.Int16 : _value.Int16 + _buttonIncrement.Int16)
                : (_value.Int16 - _minValue.Int16 < _buttonIncrement.Int16 ? _minValue.Int16 : _value.Int16 - _buttonIncrement.Int16);
            SetValue<i16>(clamped.Int16);
        break;
        case ValueType::Int32:
            clamped.Int32 = increase
                ? (_maxValue.Int32 - _value.Int32 < _buttonIncrement.Int32 ? _maxValue.Int32 : _value.Int32 + _buttonIncrement.Int32)
                : (_value.Int32 - _minValue.Int32 < _buttonIncrement.Int32 ? _minValue.Int32 : _value.Int32 - _buttonIncrement.Int32);
            SetValue<i32>(clamped.Int32);
        break;
        case ValueType::Int64:
            clamped.Int64 = increase
                ? (_maxValue.Int64 - _value.Int64 < _buttonIncrement.Int64 ? _maxValue.Int64 : _value.Int64 + _buttonIncrement.Int64)
                : (_value.Int64 - _minValue.Int64 < _buttonIncrement.Int64 ? _minValue.Int64 : _value.Int64 - _buttonIncrement.Int64);
            SetValue<i64>(clamped.Int64);
        break;
        case ValueType::UInt8:
            clamped.UInt8 = increase
                ? (_maxValue.UInt8 - _value.UInt8 < _buttonIncrement.UInt8 ? _maxValue.UInt8 : _value.UInt8 + _buttonIncrement.UInt8)
                : (_value.UInt8 - _minValue.UInt8 < _buttonIncrement.UInt8 ? _minValue.UInt8 : _value.UInt8 - _buttonIncrement.UInt8);
            SetValue<u8>(clamped.UInt8);
        break;
        case ValueType::UInt16:
            clamped.UInt16 = increase
                ? (_maxValue.UInt16 - _value.UInt16 < _buttonIncrement.UInt16 ? _maxValue.UInt16 : _value.UInt16 + _buttonIncrement.UInt16)
                : (_value.UInt16 - _minValue.UInt16 < _buttonIncrement.UInt16 ? _minValue.UInt16 : _value.UInt16 - _buttonIncrement.UInt16);
            SetValue<u16>(clamped.UInt16);
        break;
        case ValueType::UInt32:
            clamped.UInt32 = increase
                ? (_maxValue.UInt32 - _value.UInt32 < _buttonIncrement.UInt32 ? _maxValue.UInt32 : _value.UInt32 + _buttonIncrement.UInt32)
                : (_value.UInt32 - _minValue.UInt32 < _buttonIncrement.UInt32 ? _minValue.UInt32 : _value.UInt32 - _buttonIncrement.UInt32);
            SetValue<u32>(clamped.UInt32);
        break;
        case ValueType::UInt64:
            clamped.UInt64 = increase
                ? (_maxValue.UInt64 - _value.UInt64 < _buttonIncrement.UInt64 ? _maxValue.UInt64 : _value.UInt64 + _buttonIncrement.UInt64)
                : (_value.UInt64 - _minValue.UInt64 < _buttonIncrement.UInt64 ? _minValue.UInt64 : _value.UInt64 - _buttonIncrement.UInt64);
            SetValue<u64>(clamped.UInt64);
        break;
        case ValueType::Float32:
            clamped.Float32 = increase
                ? (_maxValue.Float32 - _value.Float32 < _buttonIncrement.Float32 ? _maxValue.Float32 : _value.Float32 + _buttonIncrement.Float32)
                : (_value.Float32 - _minValue.Float32 < _buttonIncrement.Float32 ? _minValue.Float32 : _value.Float32 - _buttonIncrement.Float32);
            SetValue<f32>(clamped.Float32);
        break;
        case ValueType::Float64:
            clamped.Float64 = increase
                ? (_maxValue.Float64 - _value.Float64 < _buttonIncrement.Float64 ? _maxValue.Float64 : _value.Float64 + _buttonIncrement.Float64)
                : (_value.Float64 - _minValue.Float64 < _buttonIncrement.Float64 ? _minValue.Float64 : _value.Float64 - _buttonIncrement.Float64);
            SetValue<f64>(clamped.Float64);
        break;
        default:
            SetValue<bool>(increase);
        break;
    }
}

void RetroFuturaGUI::Slider::SetCornerRadii(const glm::vec4& radii)
{
    if(_background)
        _background->SetCornerRadii(radii);

    if(_border)
        _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::Slider::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
}

bool RetroFuturaGUI::Slider::isInsideIndicator(const glm::vec2& mousePos)
{
    return isPointInsideRect(mousePos, glm::vec3(_indicatorBackground->GetSize(), 0.0f), _indicatorBackground->GetPosition(), _indicatorBackground->GetRotation());
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
    bool isMouseInsideIndicator = isPointInsideRect(mousePos, glm::vec3(_indicatorBackground->GetSize(), 0.0f), _indicatorBackground->GetPosition(), _indicatorBackground->GetRotation());
    bool isMouseInsideButtonLower = _useSliderButtons && _buttonLower && isPointInsideRect(mousePos, _buttonLower->GetSize(), _buttonLower->GetPosition(), _buttonLower->GetRotation());
    bool isMouseInsideButtonHigher = _useSliderButtons && _buttonHigher && isPointInsideRect(mousePos, _buttonHigher->GetSize(), _buttonHigher->GetPosition(), _buttonHigher->GetRotation());

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
            setIndicatorColors(ColorState::Enabled);
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

        if(isMouseInsideIndicator)
            setIndicatorColors(ColorState::Hover);
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);

        if(isMouseInsideIndicator)
        {
            setIndicatorColors(ColorState::Clicked);
            _isDraggingIndicator = true;
        }
        else if(!isMouseInsideButtonLower && !isMouseInsideButtonHigher)
        {
            setValueFromMousePosition(mousePos);
            _onValueChanged.Emit();
            _onValueChangedAsync.EmitAsync();
        }

        _previousIndicatorPosition = mousePos;
    }
    else if(isMouseButtonPressed && _wasClicked) //held (drag)
    {
        if(_isDraggingIndicator // consider indicator might be rotated
            && (_previousIndicatorPosition.x != mousePos.x || _previousIndicatorPosition.y != mousePos.y))
        {
            setValueFromMousePosition(mousePos);
            _previousIndicatorPosition = mousePos;
            _onValueChanged.Emit();
            _onValueChangedAsync.EmitAsync();
        }
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();
        _isDraggingIndicator = false;

        if(isHovering)
        {
            setColors(ColorState::Hover);

            if(isMouseInsideIndicator)
                setIndicatorColors(ColorState::Hover);
        }
        else
        {
            setColors(ColorState::Enabled);

            if(isMouseInsideIndicator)
                setIndicatorColors(ColorState::Enabled);
        }
    }

    _wasClicked = isMouseButtonPressed;
}