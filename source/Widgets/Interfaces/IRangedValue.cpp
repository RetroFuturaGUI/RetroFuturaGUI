#include "IRangedValue.hpp"
#include "Rectangle.hpp"
#include <memory>

void RetroFuturaGUI::IRangedValue::Connect_OnValueChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueChangedAsync.Connect(slot);
    else
        _onValueChanged.Connect(slot);
}

void RetroFuturaGUI::IRangedValue::Connect_OnValueSet(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueSetAsync.Connect(slot);
    else
        _onValueSet.Connect(slot);
}

void RetroFuturaGUI::IRangedValue::Disconnect_OnValueChanged(const typename Signal<>::Slot& slot)
{
    _onValueChanged.Disconnect(slot);
    _onValueChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::IRangedValue::Disconnect_OnValueSet(const typename Signal<>::Slot& slot)
{
    _onValueSet.Disconnect(slot);
    _onValueSetAsync.Disconnect(slot);
}

f32 RetroFuturaGUI::IRangedValue::getValueFraction() const
{
    f32
        value { 0.0f },
        minValue { 0.0f },
        maxValue { 1.0f };

    switch(_valueType)
    {
        case ValueType::Int8:
            value = static_cast<f32>(_value.Int8);
            minValue = static_cast<f32>(_minValue.Int8);
            maxValue = static_cast<f32>(_maxValue.Int8);
        break;
        case ValueType::Int16:
            value = static_cast<f32>(_value.Int16);
            minValue = static_cast<f32>(_minValue.Int16);
            maxValue = static_cast<f32>(_maxValue.Int16);
        break;
        case ValueType::Int32:
            value = static_cast<f32>(_value.Int32);
            minValue = static_cast<f32>(_minValue.Int32);
            maxValue = static_cast<f32>(_maxValue.Int32);
        break;
        case ValueType::Int64:
            value = static_cast<f32>(_value.Int64);
            minValue = static_cast<f32>(_minValue.Int64);
            maxValue = static_cast<f32>(_maxValue.Int64);
        break;
        case ValueType::UInt8:
            value = static_cast<f32>(_value.UInt8);
            minValue = static_cast<f32>(_minValue.UInt8);
            maxValue = static_cast<f32>(_maxValue.UInt8);
        break;
        case ValueType::UInt16:
            value = static_cast<f32>(_value.UInt16);
            minValue = static_cast<f32>(_minValue.UInt16);
            maxValue = static_cast<f32>(_maxValue.UInt16);
        break;
        case ValueType::UInt32:
            value = static_cast<f32>(_value.UInt32);
            minValue = static_cast<f32>(_minValue.UInt32);
            maxValue = static_cast<f32>(_maxValue.UInt32);
        break;
        case ValueType::UInt64:
            value = static_cast<f32>(_value.UInt64);
            minValue = static_cast<f32>(_minValue.UInt64);
            maxValue = static_cast<f32>(_maxValue.UInt64);
        break;
        case ValueType::Float32:
            value = _value.Float32;
            minValue = _minValue.Float32;
            maxValue = _maxValue.Float32;
        break;
        case ValueType::Float64:
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
    return range != 0.0f ? (value - minValue) / range : 0.0f;
}

void RetroFuturaGUI::IRangedValue::EnableIndicator(const bool value)
{
    _useIndicator = value;

    if(_useIndicator && !_indicatorBackground)
    {
        _indicatorBackground = std::make_unique<Rectangle>(_elementProjection);
        _indicatorBackground->SetRectangleMode(RectangleMode::Plane);
        setIndicatorBackgroundColors();
    }

    setIndicatorPosition();
}

void RetroFuturaGUI::IRangedValue::SetIndicatorType(const IndicatorType type)
{
    _indicatorType = type;
    const bool isCircle { _indicatorType == IndicatorType::Circle };

    if(_indicatorBackground)
    {
        if(isCircle)
        {
            _indicatorBackground->SetShaderFeatures(RoundedCorners);
            _indicatorBackground->SetCornerRadii(glm::vec4(_indicatorBackground->GetSize().x * 0.5f));
        }
        else
        {
            _indicatorBackground->SetShaderFeatures(0);
            _indicatorBackground->SetCornerRadii(glm::vec4(0.0f));
        }
    }

    if(_indicatorBorder)
    {
        if(isCircle)
        {
            _indicatorBorder->SetShaderFeatures(RoundedCorners);
            _indicatorBorder->SetCornerRadii(glm::vec4(_indicatorBorder->GetSize().x * 0.5f));
        }
        else
        {
            _indicatorBorder->SetShaderFeatures(0);
            _indicatorBorder->SetCornerRadii(glm::vec4(0.0f));
        }
    }
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _indicatorBackgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _indicatorBackgroundColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _indicatorBackgroundColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _indicatorBackgroundColorEnabled.assign(colors.begin(), colors.end());
    }

    setIndicatorBackgroundColors();
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _indicatorBorderColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _indicatorBorderColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _indicatorBorderColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _indicatorBorderColorEnabled.assign(colors.begin(), colors.end());
    }

    setIndicatorBorderColors();
}

void RetroFuturaGUI::IRangedValue::SetIndicatorCornerRadii(const glm::vec4& radii)
{
    if(_indicatorBackground)
        _indicatorBackground->SetCornerRadii(radii);

    if(_indicatorBorder)
        _indicatorBorder->SetCornerRadii(radii);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFillType(const FillType fillType)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFillType(fillType);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientOffset(const f32 gradientOffset)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientDegree(const f32 degree)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_indicatorBackground)
        _indicatorBackground->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotColor(const glm::vec4& color)
{
    if(_indicatorBackground)
        _indicatorBackground->SetDotColor(color);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotDistance(const f32 distance)
{
    if(_indicatorBackground)
        _indicatorBackground->SetDotDistance(distance);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotSizeTransferDegree(const f32 degree)
{
    if(_indicatorBackground)
        _indicatorBackground->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _indicatorBackgroundDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(_indicatorBackground)
        _indicatorBackground->SetDotRadiusTransfer(_indicatorBackgroundDotRadiusTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_indicatorBackground)
        _indicatorBackground->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogAlpha(const f32 alpha)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFogAlpha(alpha);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogSpeed(const f32 speed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFogSpeed(speed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogDensity(std::span<f32> density)
{
    _indicatorBackgroundFogDensity.assign(density.begin(), density.end());

    if(_indicatorBackground)
        _indicatorBackground->SetFogDensity(_indicatorBackgroundFogDensity);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogClearing(const f32 clearing)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFogClearing(clearing);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderFillType(const FillType fillType)
{
    if(_indicatorBorder)
        _indicatorBorder->SetFillType(fillType);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientOffset(const f32 gradientOffset)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientDegree(const f32 degree)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorWindowBorderImageTextureID(const u32 textureID)
{
    if(_indicatorBorder)
        _indicatorBorder->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotColor(const glm::vec4& color)
{
    if(_indicatorBorder)
        _indicatorBorder->SetDotColor(color);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotDistance(const f32 distance)
{
    if(_indicatorBorder)
        _indicatorBorder->SetDotDistance(distance);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotSizeTransferDegree(const f32 degree)
{
    if(_indicatorBorder)
        _indicatorBorder->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _indicatorBorderDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(_indicatorBorder)
        _indicatorBorder->SetDotRadiusTransfer(_indicatorBorderDotRadiusTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_indicatorBorder)
        _indicatorBorder->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBorder)
        _indicatorBorder->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderWidth(const f32 borderWidth)
{
    if(_indicatorBorder)
        _indicatorBorder->SetBorderWidth(borderWidth);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGaps(std::span<BorderGap> gaps)
{
    if(_indicatorBorder)
        _indicatorBorder->SetBorderGaps(gaps);
}

void RetroFuturaGUI::IRangedValue::EnableGraph(const bool value)
{
    _useGraph = value;

    if(_useGraph && !_graph)
    {
        _graph = std::make_unique<Rectangle>(_elementProjection);
        _graph->SetRectangleMode(RectangleMode::Plane);
        setGraphColorsApply();
    }

    setGraphPosition();
}

void RetroFuturaGUI::IRangedValue::SetGraphMode(const GraphMode mode)
{
    _graphMode = mode;

    if(!_graph)
        return;

    const u32 features = _graphMode == GraphMode::Wave
        ? (_graph->GetShaderFeatures() | Wave)
        : (_graph->GetShaderFeatures() & ~Wave);

    _graph->SetShaderFeatures(features);
    // TODO: GraphMode::Wave has no distinct rendering path yet — it draws identically to Bar.
}

void RetroFuturaGUI::IRangedValue::SetGraphColors(std::span<glm::vec4> colors, const ColorState state)
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

    setGraphColorsApply();
}

void RetroFuturaGUI::IRangedValue::SetGraphCornerRadii(const glm::vec4& radii)
{
    if(_graph)
        _graph->SetCornerRadii(radii);
}

void RetroFuturaGUI::IRangedValue::SetGraphWidth(const f32 width)
{
    _graphWidth = width;
    setGraphPosition();
}

void RetroFuturaGUI::IRangedValue::SetGraphFillType(const FillType fillType)
{
    if(_graph)
        _graph->SetFillType(fillType);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientOffset(const f32 gradientOffset)
{
    if(_graph)
        _graph->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_graph)
        _graph->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientDegree(const f32 degree)
{
    if(_graph)
        _graph->SetGradientDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_graph)
        _graph->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_graph)
        _graph->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotColor(const glm::vec4& color)
{
    if(_graph)
        _graph->SetDotColor(color);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotDistance(const f32 distance)
{
    if(_graph)
        _graph->SetDotDistance(distance);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotSizeTransferDegree(const f32 degree)
{
    if(_graph)
        _graph->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _graphDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(_graph)
        _graph->SetDotRadiusTransfer(_graphDotRadiusTransfer);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_graph)
        _graph->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotAnimationSpeed(const f32 animationSpeed)
{
    if(_graph)
        _graph->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogAlpha(const f32 alpha)
{
    if(_graph)
        _graph->SetFogAlpha(alpha);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogSpeed(const f32 speed)
{
    if(_graph)
        _graph->SetFogSpeed(speed);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogDensity(std::span<f32> density)
{
    _graphFogDensity.assign(density.begin(), density.end());

    if(_graph)
        _graph->SetFogDensity(_graphFogDensity);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogClearing(const f32 clearing)
{
    if(_graph)
        _graph->SetFogClearing(clearing);
}

void RetroFuturaGUI::IRangedValue::SetOrientation(const Orientation orientation)
{
    _orientation = orientation;
}

glm::vec3 RetroFuturaGUI::IRangedValue::orientedRotation(const glm::vec3& rotation) const
{
    return _orientation == Orientation::Vertical
        ? rotation + glm::vec3(0.0f, 0.0f, -90.0f)
        : rotation;
}

void RetroFuturaGUI::IRangedValue::setIndicatorPosition()
{
    if(!_track || !_indicatorBackground)
        return;

    const f32
        fraction { getValueFraction() },
        trackLength { _track->GetSize().x },
        indicatorLength { _indicatorBackground->GetSize().x },
        travelRange { trackLength - indicatorLength > 0.0f ? trackLength - indicatorLength : 0.0f },
        indicatorSliderPosition { indicatorLength * 0.5f + fraction * travelRange };

    // The indicator always travels along the track's local x-axis.
    // Rotate that local offset by the track's rotation to place it correctly in world space.
    const glm::vec2 localOffset(indicatorSliderPosition - trackLength * 0.5f, 0.0f);
    const f32 radians = glm::radians(_track->GetRotation().z);
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    const glm::vec3 position
    (
        _track->GetPosition().x + rotatedOffset.x,
        _track->GetPosition().y + rotatedOffset.y,
        _track->GetPosition().z + 0.02f
    );

    _indicatorBackground->SetPosition(position);
    _indicatorBackground->SetRotation(_track->GetRotation());

    if(_indicatorBorder)
    {
        _indicatorBorder->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
        _indicatorBorder->SetRotation(_track->GetRotation());
    }
}

void RetroFuturaGUI::IRangedValue::setGraphSize()
{
    if(!_track || !_graph)
        return;

    const f32
        trackWidth { _track->GetSize().x },
        trackHeight { _track->GetSize().y },
        graphWidth { trackWidth * getValueFraction() },
        graphHeight { _graphWidth > 0.0f ? (_graphWidth < trackHeight ? _graphWidth : trackHeight) : trackHeight };

    _graph->SetSize(glm::vec2(graphWidth, graphHeight));
}

void RetroFuturaGUI::IRangedValue::setGraphPosition()
{
    if(!_track || !_graph)
        return;

    setGraphSize();

    const f32
        trackWidth { _track->GetSize().x },
        graphWidth { _graph->GetSize().x };

    // The graph always grows along the track's local x-axis, anchored to its left edge.
    const glm::vec2 localOffset(graphWidth * 0.5f - trackWidth * 0.5f, 0.0f);
    const f32 radians = glm::radians(_track->GetRotation().z);
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    const glm::vec3 position
    (
        _track->GetPosition().x + rotatedOffset.x,
        _track->GetPosition().y + rotatedOffset.y,
        _track->GetPosition().z + 0.02f
    );

    _graph->SetPosition(position);
    _graph->SetRotation(_track->GetRotation());
}

void RetroFuturaGUI::IRangedValue::setValueFromMousePosition(const glm::vec2& mousePos)
{
    if(!_track || !_indicatorBackground)
        return;

    // Revert the track's rotation to bring the mouse position into the track's local space where it always runs along the local X axis
    const glm::vec2 translated { mousePos - glm::vec2(_track->GetPosition().x, _track->GetPosition().y) };
    const f32 radians { glm::radians(-_track->GetRotation().z) };
    const glm::vec2 localMouse
    (
        translated.x * cos(radians) - translated.y * sin(radians),
        translated.x * sin(radians) + translated.y * cos(radians)
    );

    const f32
        trackLength { _track->GetSize().x },
        indicatorLength { _indicatorBackground->GetSize().x },
        travelRange { trackLength - indicatorLength > 0.0f ? trackLength - indicatorLength : 0.0f },
        trackNearEdge { -trackLength * 0.5f },
        mouseOffsetOnAxis { localMouse.x - trackNearEdge - indicatorLength * 0.5f },
        clampedOffset { mouseOffsetOnAxis < 0.0f ? 0.0f : (mouseOffsetOnAxis > travelRange ? travelRange : mouseOffsetOnAxis) },
        fraction { travelRange != 0.0f ? clampedOffset / travelRange : 0.0f };

    switch(_valueType)
    {
        case ValueType::Int8:
            _value.Int8 = static_cast<i8>(_minValue.Int8 + fraction * (_maxValue.Int8 - _minValue.Int8));
        break;
        case ValueType::Int16:
            _value.Int16 = static_cast<i16>(_minValue.Int16 + fraction * (_maxValue.Int16 - _minValue.Int16));
        break;
        case ValueType::Int32:
            _value.Int32 = static_cast<i32>(_minValue.Int32 + fraction * (_maxValue.Int32 - _minValue.Int32));
        break;
        case ValueType::Int64:
            _value.Int64 = static_cast<i64>(_minValue.Int64 + fraction * (_maxValue.Int64 - _minValue.Int64));
        break;
        case ValueType::UInt8:
            _value.UInt8 = static_cast<u8>(_minValue.UInt8 + fraction * (_maxValue.UInt8 - _minValue.UInt8));
        break;
        case ValueType::UInt16:
            _value.UInt16 = static_cast<u16>(_minValue.UInt16 + fraction * (_maxValue.UInt16 - _minValue.UInt16));
        break;
        case ValueType::UInt32:
            _value.UInt32 = static_cast<u32>(_minValue.UInt32 + fraction * (_maxValue.UInt32 - _minValue.UInt32));
        break;
        case ValueType::UInt64:
            _value.UInt64 = static_cast<u64>(_minValue.UInt64 + fraction * (_maxValue.UInt64 - _minValue.UInt64));
        break;
        case ValueType::Float32:
            _value.Float32 = _minValue.Float32 + fraction * (_maxValue.Float32 - _minValue.Float32);
        break;
        case ValueType::Float64:
            _value.Float64 = _minValue.Float64 + static_cast<f64>(fraction) * (_maxValue.Float64 - _minValue.Float64);
        break;
        default: // Bool
            _value.Bool = fraction >= 0.5f;
        break;
    }

    setIndicatorPosition();
}

void RetroFuturaGUI::IRangedValue::setIndicatorColors(const ColorState state)
{
    _indicatorBackgroundColorState = state;
    _indicatorBorderColorState = state;
    setIndicatorBackgroundColors();
    setIndicatorBorderColors();
}

void RetroFuturaGUI::IRangedValue::setGraphColors(const ColorState state)
{
    _graphColorState = state;
    setGraphColorsApply();
}

void RetroFuturaGUI::IRangedValue::setIndicatorBackgroundColors()
{
    if(!_indicatorBackground)
        return;

    switch(_indicatorBackgroundColorState)
    {
        case ColorState::Enabled:
            _indicatorBackground->SetColors(_indicatorBackgroundColorEnabled);
        break;
        case ColorState::Clicked:
            _indicatorBackground->SetColors(_indicatorBackgroundColorClicked);
        break;
        case ColorState::Hover:
            _indicatorBackground->SetColors(_indicatorBackgroundColorHover);
        break;
        default: //Disabled
            _indicatorBackground->SetColors(_indicatorBackgroundColorDisabled);
    }
}

void RetroFuturaGUI::IRangedValue::setIndicatorBorderColors()
{
    if(!_indicatorBorder)
        return;

    switch(_indicatorBorderColorState)
    {
        case ColorState::Enabled:
            _indicatorBorder->SetColors(_indicatorBorderColorEnabled);
        break;
        case ColorState::Clicked:
            _indicatorBorder->SetColors(_indicatorBorderColorClicked);
        break;
        case ColorState::Hover:
            _indicatorBorder->SetColors(_indicatorBorderColorHover);
        break;
        default: //Disabled
            _indicatorBorder->SetColors(_indicatorBorderColorDisabled);
    }
}

void RetroFuturaGUI::IRangedValue::setGraphColorsApply()
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

void RetroFuturaGUI::IRangedValue::drawIndicator()
{
    if(!_useIndicator)
        return;

    if(_indicatorBackground)
        _indicatorBackground->Draw();

    if(_indicatorBorder)
        _indicatorBorder->Draw();
}

void RetroFuturaGUI::IRangedValue::drawGraph()
{
    if(!_useGraph || !_graph)
        return;

    _graph->Draw(); // TODO: GraphMode::Wave currently draws identically to GraphMode::Bar — wave geometry/shader not implemented yet.
}