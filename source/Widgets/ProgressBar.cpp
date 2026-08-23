#include "ProgressBar.hpp"
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

RetroFuturaGUI::ProgressBar::ProgressBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ProgressBar;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _graph = std::make_unique<Rectangle>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if (_graph)
        _graph->SetRectangleMode(RectangleMode::Plane);
}

void RetroFuturaGUI::ProgressBar::Connect_OnValueChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueChangedAsync.Connect(slot);
    else
        _onValueChanged.Connect(slot);
}

void RetroFuturaGUI::ProgressBar::Connect_OnValueSet(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueSetAsync.Connect(slot);
    else
        _onValueSet.Connect(slot);
}

void RetroFuturaGUI::ProgressBar::Disconnect_OnValueChanged(const typename Signal<>::Slot& slot)
{
    _onValueChanged.Disconnect(slot);
    _onValueChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::ProgressBar::Disconnect_OnValueSet(const typename Signal<>::Slot& slot)
{
    _onValueSet.Disconnect(slot);
    _onValueSetAsync.Disconnect(slot);
}

void  RetroFuturaGUI::ProgressBar::Draw()
{
    interact();

    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

    if (_graph)
        _graph->Draw(); // TODO: GraphMode::Wave currently draws identically to GraphMode::Bar — wave geometry/shader not implemented yet.

    if(!_useIndicator)
        return;

    if(_indicator)
        _indicator->Draw();
}

void RetroFuturaGUI::ProgressBar::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if (_background)
        _background->SetPosition(position);

    if (_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    setGraphPosition();
}

void RetroFuturaGUI::ProgressBar::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if (_background)
        _background->SetSize(size);

    if (_border)
        _border->SetSize(size);

    setGraphSize();
    setGraphPosition();
}

void RetroFuturaGUI::ProgressBar::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if (_background)
        _background->SetRotation(rotation);

    if (_border)
        _border->SetRotation(rotation);

    setGraphPosition();
}

void RetroFuturaGUI::ProgressBar::SetGraphMode(const GraphMode mode)
{
    _graphMode = mode;

    if(!_graph)
        return;

    const u32 features = _graphMode == GraphMode::Wave
        ? (_graph->GetShaderFeatures() | Wave)
        : (_graph->GetShaderFeatures() & ~Wave);

    _graph->SetShaderFeatures(features);
}

void RetroFuturaGUI::ProgressBar::SetGraphWidth(const f32 width)
{
    _graphWidth = width;
    setGraphPosition();
}

void RetroFuturaGUI::ProgressBar::setGraphSize()
{
    if(!_background || !_graph)
        return;

    calculateGraphProgressBarPosition();

    const f32
        trackWidth { _background->GetSize().x },
        trackHeight { _background->GetSize().y },
        graphWidth { trackWidth * _graphValueFraction },
        graphHeight { _graphWidth > 0.0f ? (_graphWidth < trackHeight ? _graphWidth : trackHeight) : trackHeight };

    _graph->SetSize(glm::vec2(graphWidth, graphHeight));
}

void RetroFuturaGUI::ProgressBar::setGraphPosition()
{
    if(!_background || !_graph)
        return;

    setGraphSize();

    const f32
        trackWidth { _background->GetSize().x },
        graphWidth { _graph->GetSize().x };

    // The graph always grows along the background's local x-axis, anchored to its left edge.
    // Rotate that local offset by the widget's rotation to place it correctly in world space.
    const glm::vec2 localOffset(graphWidth * 0.5f - trackWidth * 0.5f, 0.0f);
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

    _graph->SetPosition(position);
    _graph->SetRotation(_rotation);

    setIndicatorPosition();
}

void RetroFuturaGUI::ProgressBar::calculateGraphProgressBarPosition()
{
    f32
        value { 0.0f },
        minValue { 0.0f },
        maxValue { 1.0f };

    switch(_valueType)
    {
        case ProgressBarValueType::Int8:
            value = static_cast<f32>(_value.Int8);
            minValue = static_cast<f32>(_minValue.Int8);
            maxValue = static_cast<f32>(_maxValue.Int8);
        break;
        case ProgressBarValueType::Int16:
            value = static_cast<f32>(_value.Int16);
            minValue = static_cast<f32>(_minValue.Int16);
            maxValue = static_cast<f32>(_maxValue.Int16);
        break;
        case ProgressBarValueType::Int32:
            value = static_cast<f32>(_value.Int32);
            minValue = static_cast<f32>(_minValue.Int32);
            maxValue = static_cast<f32>(_maxValue.Int32);
        break;
        case ProgressBarValueType::Int64:
            value = static_cast<f32>(_value.Int64);
            minValue = static_cast<f32>(_minValue.Int64);
            maxValue = static_cast<f32>(_maxValue.Int64);
        break;
        case ProgressBarValueType::UInt8:
            value = static_cast<f32>(_value.UInt8);
            minValue = static_cast<f32>(_minValue.UInt8);
            maxValue = static_cast<f32>(_maxValue.UInt8);
        break;
        case ProgressBarValueType::UInt16:
            value = static_cast<f32>(_value.UInt16);
            minValue = static_cast<f32>(_minValue.UInt16);
            maxValue = static_cast<f32>(_maxValue.UInt16);
        break;
        case ProgressBarValueType::UInt32:
            value = static_cast<f32>(_value.UInt32);
            minValue = static_cast<f32>(_minValue.UInt32);
            maxValue = static_cast<f32>(_maxValue.UInt32);
        break;
        case ProgressBarValueType::UInt64:
            value = static_cast<f32>(_value.UInt64);
            minValue = static_cast<f32>(_minValue.UInt64);
            maxValue = static_cast<f32>(_maxValue.UInt64);
        break;
        case ProgressBarValueType::Float32:
            value = _value.Float32;
            minValue = _minValue.Float32;
            maxValue = _maxValue.Float32;
        break;
        case ProgressBarValueType::Float64:
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

    const f32 range { maxValue - minValue };
    _graphValueFraction = range != 0.0f ? (value - minValue) / range : 0.0f;
}

void RetroFuturaGUI::ProgressBar::SetGraphColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Disabled:
            _graphColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _graphColorHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _graphColorClicked.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _graphColorEnabled.assign(colors.begin(), colors.end());
    }

    setGraphColors();
}

void RetroFuturaGUI::ProgressBar::SetCornerRadii(const glm::vec4& radii)
{
    if(_background)
        _background->SetCornerRadii(radii);

    if(_border)
        _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::ProgressBar::SetGraphCornerRadii(const glm::vec4& radii)
{
    if(_graph)
        _graph->SetCornerRadii(radii);
}

void RetroFuturaGUI::ProgressBar::SetGraphFillType(const FillType fillType)
{
    if(_graph)
        _graph->SetFillType(fillType);
}

void RetroFuturaGUI::ProgressBar::SetGraphGradientOffset(const f32 gradientOffset)
{
    if(_graph)
        _graph->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::ProgressBar::SetGraphGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_graph)
        _graph->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::ProgressBar::SetGraphGradientDegree(const f32 degree)
{
    if(_graph)
        _graph->SetGradientDegree(degree);
}

void RetroFuturaGUI::ProgressBar::SetGraphGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_graph)
        _graph->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::ProgressBar::SetGraphWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_graph)
        _graph->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::ProgressBar::SetGraphDotColor(const glm::vec4& color)
{
    if(_graph)
        _graph->SetDotColor(color);
}

void RetroFuturaGUI::ProgressBar::SetGraphDotDistance(const f32 distance)
{
    if(_graph)
        _graph->SetDotDistance(distance);
}

void RetroFuturaGUI::ProgressBar::SetGraphDotSizeTransferDegree(const f32 degree)
{
    if(_graph)
        _graph->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::ProgressBar::SetGraphDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _graphDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(_graph)
        _graph->SetDotRadiusTransfer(_graphDotRadiusTransfer);
}

void RetroFuturaGUI::ProgressBar::SetGraphDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_graph)
        _graph->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::ProgressBar::SetGraphDotAnimationSpeed(const f32 animationSpeed)
{
    if(_graph)
        _graph->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::ProgressBar::SetGraphFogAlpha(const f32 alpha)
{
    if(_graph)
        _graph->SetFogAlpha(alpha);
}

void RetroFuturaGUI::ProgressBar::SetGraphFogSpeed(const f32 speed)
{
    if(_graph)
        _graph->SetFogSpeed(speed);
}

void RetroFuturaGUI::ProgressBar::SetGraphFogDensity(std::span<f32> density)
{
    _graphFogDensity.assign(density.begin(), density.end());

    if(_graph)
        _graph->SetFogDensity(_graphFogDensity);
}

void RetroFuturaGUI::ProgressBar::SetGraphFogClearing(const f32 clearing)
{
    if(_graph)
        _graph->SetFogClearing(clearing);
}

void RetroFuturaGUI::ProgressBar::EnableIndicator(const bool value)
{
    _useIndicator = value;

    if(_useIndicator && !_indicator)
    {
        _indicator = std::make_unique<Rectangle>(&_projection);
        _indicator->SetRectangleMode(RectangleMode::Plane);
        setIndicatorShape();
        setIndicatorColors();
    }

    setGraphPosition();
}

void RetroFuturaGUI::ProgressBar::SetIndicatorType(const IndicatorType type)
{
    _indicatorType = type;
    setIndicatorShape();
}

void RetroFuturaGUI::ProgressBar::SetIndicatorSize(const f32 size)
{
    _indicatorSize = size;
    setIndicatorShape();
}

void RetroFuturaGUI::ProgressBar::SetIndicatorColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Disabled:
            _indicatorColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _indicatorColorHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _indicatorColorClicked.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _indicatorColorEnabled.assign(colors.begin(), colors.end());
    }

    setIndicatorColors();
}

void RetroFuturaGUI::ProgressBar::setGraphColors()
{
    if(!_graph)
        return;

    switch(_graphColorState)
    {
        case ColorState::Enabled:
            _graph->SetColors(_graphColorEnabled);
        break;
        case ColorState::Clicked:
            _graph->SetColors(_graphColorClicked);
        break;
        case ColorState::Hover:
            _graph->SetColors(_graphColorHover);
        break;
        default: //Disabled
            _graph->SetColors(_graphColorDisabled);
    }
}

void RetroFuturaGUI::ProgressBar::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
}

void RetroFuturaGUI::ProgressBar::setGraphColors(const ColorState state)
{
    _graphColorState = state;
    setGraphColors();
}

void RetroFuturaGUI::ProgressBar::setIndicatorPosition()
{
    if(!_background || !_graph || !_indicator)
        return;

    const f32
        trackWidth { _background->GetSize().x },
        graphWidth { _graph->GetSize().x };

    // The indicator sits on the graph's current leading (right) edge and travels with it.
    const glm::vec2 localOffset(graphWidth - trackWidth * 0.5f, 0.0f);
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
        _background->GetPosition().z + 0.03f
    );

    _indicator->SetPosition(position);
    _indicator->SetRotation(_rotation);
}

void RetroFuturaGUI::ProgressBar::setIndicatorShape()
{
    if(!_indicator)
        return;

    _indicator->SetSize(glm::vec2(_indicatorSize, _indicatorSize));

    if(_indicatorType == IndicatorType::Circle)
    {
        _indicator->SetShaderFeatures(RoundedCorners);
        _indicator->SetCornerRadii(glm::vec4(_indicatorSize * 0.5f));
    }
    else
    {
        _indicator->SetShaderFeatures(0);
        _indicator->SetCornerRadii(glm::vec4(0.0f));
    }
}

void RetroFuturaGUI::ProgressBar::setIndicatorColors()
{
    if(!_indicator)
        return;

    switch(_indicatorColorState)
    {
        case ColorState::Enabled:
            _indicator->SetColors(_indicatorColorEnabled);
        break;
        case ColorState::Clicked:
            _indicator->SetColors(_indicatorColorClicked);
        break;
        case ColorState::Hover:
            _indicator->SetColors(_indicatorColorHover);
        break;
        default: //Disabled
            _indicator->SetColors(_indicatorColorDisabled);
    }
}

void RetroFuturaGUI::ProgressBar::setIndicatorColors(const ColorState state)
{
    _indicatorColorState = state;
    setIndicatorColors();
}

bool RetroFuturaGUI::ProgressBar::isInsideGraph(const glm::vec2& mousePos)
{
    const glm::vec2 graphPosition(_graph->GetPosition().x, _graph->GetPosition().y);
    return isPointInsideRect(mousePos, glm::vec3(_graph->GetSize(), 0.0f), _graph->GetPosition(), _graph->GetRotation());
}

void RetroFuturaGUI::ProgressBar::interact()
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
    bool isMouseInsideGraph = isPointInsideRect(mousePos, glm::vec3(_graph->GetSize(), 0.0f), _graph->GetPosition(), _graph->GetRotation());

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
            setGraphColors(ColorState::Enabled);
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

        if(isMouseInsideGraph)
            setGraphColors(ColorState::Hover);
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
        setGraphColors(ColorState::Clicked);
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
        {
            setColors(ColorState::Hover);

            if(isMouseInsideGraph)
                setGraphColors(ColorState::Hover);
        }
        else
        {
            setColors(ColorState::Enabled);

            if(isMouseInsideGraph)
                setGraphColors(ColorState::Enabled);
        }
    }

    _wasClicked = isMouseButtonPressed;
}
