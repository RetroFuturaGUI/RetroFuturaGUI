#include "IRangedValue.hpp"
#include <memory>


RetroFuturaGUI::IRangedValue::IRangedValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _indicator.SetTransitionSettings(&_transitionSettings);
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    _elementProjection = projection;
}

RetroFuturaGUI::Background& RetroFuturaGUI::IRangedValue::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::IRangedValue::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Indicator& RetroFuturaGUI::IRangedValue::GetIndicator()
{
    return _indicator;
}

const RetroFuturaGUI::Indicator& RetroFuturaGUI::IRangedValue::GetIndicator() const
{
    return _indicator;
}

RetroFuturaGUI::Border& RetroFuturaGUI::IRangedValue::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::IRangedValue::GetBorder() const
{
    return _border;
}

void RetroFuturaGUI::IRangedValue::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    const glm::vec3 trackSize { _orientation == Orientation::Vertical
        ? glm::vec3(size.y, size.x, size.z)
        : size };

    _background.SetSize(trackSize);
    _border.SetSize(trackSize);
    setIndicatorSize();
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::orientTrack()
{
    const glm::vec3 trackRotation { orientedRotation() };
    _background.SetRotation(trackRotation);
    _border.SetRotation(trackRotation);
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetOrientation(const Orientation orientation)
{
    _orientation = orientation;
    SetSize(_size);
    orientTrack();
}

void RetroFuturaGUI::IRangedValue::SetTrackDirection(const TrackDirection direction)
{
    _trackDirection = direction;
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::ShowGraph(const bool show)
{
    _useGraph = show;

    if(_useGraph && !_graph)
    {
        _graph = std::make_unique<WavePlane>(_elementProjection);
        useTransitions({ _graph.get() });
        setGraphColorsApply();
    }

    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetGraphMode(const GraphMode mode)
{
    _graphMode = mode;

    if(!_graph)
        return;

    _graph->ShowWave(_graphMode == GraphMode::Wave);
}

void RetroFuturaGUI::IRangedValue::SetGraphWaveThickness(const f32 thickness)
{
    if(!_graph)
        return;

    _graph->SetWaveThickness(thickness);
}

void RetroFuturaGUI::IRangedValue::SetGraphWaveHeight(const f32 height)
{
    if(!_graph)
        return;

    _graph->SetWaveHeight(height);
}

void RetroFuturaGUI::IRangedValue::SetGraphWaveLength(const f32 length)
{
    if(!_graph)
        return;

    _graph->SetWaveLength(length);
}

void RetroFuturaGUI::IRangedValue::SetGraphColors(std::span<const glm::vec4> colors, const ColorState state)
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
    if(!_graph)
        return;

    _graph->SetCornerRadii(radii);
}

void RetroFuturaGUI::IRangedValue::SetGraphWidth(const f32 width)
{
    _graphWidth = width;
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetGraphFillType(const FillType fillType)
{
    if(!_graph)
        return;

    _graph->SetFillType(fillType);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientOffset(const f32 gradientOffset)
{
    if(!_graph)
        return;

    _graph->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_graph)
        return;

    _graph->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientDegree(const f32 degree)
{
    if(!_graph)
        return;

    _graph->SetGradientDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_graph)
        return;

    _graph->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphWindowBackgroundImageTextureID(const u32 textureID)
{
    if(!_graph)
        return;

    _graph->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IRangedValue::SetGraphPrimaryRasterColor(const glm::vec4& color)
{
    if(!_graph)
        return;

    _graph->SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::IRangedValue::SetGraphRasterSpacing(const f32 spacing)
{
    if(!_graph)
        return;

    _graph->SetRasterSpacing(spacing);
}

void RetroFuturaGUI::IRangedValue::SetGraphRasterDegree(const f32 degree)
{
    if(!_graph)
        return;

    _graph->SetRasterDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetGraphPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _graphPrimaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());

    if(!_graph)
        return;

    _graph->SetPrimaryRasterWidthTransfer(_graphPrimaryRasterWidthTransfer);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(!_graph)
        return;

    _graph->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IRangedValue::SetGraphRasterAnimationSpeed(const f32 animationSpeed)
{
    if(!_graph)
        return;

    _graph->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogAlpha(const f32 alpha)
{
    if(!_graph)
        return;

    _graph->SetFogAlpha(alpha);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogSpeed(const f32 speed)
{
    if(!_graph)
        return;

    _graph->SetFogSpeed(speed);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogDensity(std::span<f32> density)
{
    _graphFogDensity.assign(density.begin(), density.end());

    if(!_graph)
        return;

    _graph->SetFogDensity(_graphFogDensity);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogClearing(const f32 clearing)
{
    if(!_graph)
        return;

    _graph->SetFogClearing(clearing);
}

glm::vec3 RetroFuturaGUI::IRangedValue::orientedRotation() const
{
    return _orientation == Orientation::Vertical ? glm::vec3(0.0f, 0.0f, 90.0f) : glm::vec3(0.0f);
}

f32 RetroFuturaGUI::IRangedValue::toF32(const PrimitiveUnion value) const
{
    switch(_valueType)
    {
        case PrimitiveTypeID::Int8:
            return static_cast<f32>(value.Int8);
        case PrimitiveTypeID::Int16:
            return static_cast<f32>(value.Int16);
        case PrimitiveTypeID::Int32:
            return static_cast<f32>(value.Int32);
        case PrimitiveTypeID::Int64:
            return static_cast<f32>(value.Int64);
        case PrimitiveTypeID::UInt8:
            return static_cast<f32>(value.UInt8);
        case PrimitiveTypeID::UInt16:
            return static_cast<f32>(value.UInt16);
        case PrimitiveTypeID::UInt32:
            return static_cast<f32>(value.UInt32);
        case PrimitiveTypeID::UInt64:
            return static_cast<f32>(value.UInt64);
        case PrimitiveTypeID::Float32:
            return value.Float32;
        case PrimitiveTypeID::Float64:
            return static_cast<f32>(value.Float64);
        default: // Bool
            return value.Bool ? 1.0f : 0.0f;
    }
}

f32 RetroFuturaGUI::IRangedValue::getRangeFraction(const f64 value) const
{
    // A bool's range is always false..true, whatever its bounds hold
    if(_valueType == PrimitiveTypeID::Bool)
        return 0.0 != value ? 1.0f : 0.0f;

    // f32 rather than f32: with a large minimum, such as an absolute timestamp, f32 collapses nearby values into one fraction
    const f32
        minValue { toF32(_minValue) },
        range { toF32(_maxValue) - minValue };

    if(0.0 == range)
        return 0.0f;

    return static_cast<f32>((value - minValue) / range);
}

f32 RetroFuturaGUI::IRangedValue::toTrackFraction(const f32 rangeFraction) const
{
    return _trackDirection == TrackDirection::Inverted ? 1.0f - rangeFraction : rangeFraction;
}

void RetroFuturaGUI::IRangedValue::setValueType(const PrimitiveTypeID type)
{
    if(type == _valueType)
        return;

    const PrimitiveTypeID previousType { _valueType };
    _valueType = type;
    _minValue = convertPrimitive(_minValue, previousType, _valueType);
    _maxValue = convertPrimitive(_maxValue, previousType, _valueType);
    convertValuesToType(previousType);
}

RetroFuturaGUI::PrimitiveTypeID RetroFuturaGUI::IRangedValue::resolveValueType(const PrimitiveTypeID requestedType) const
{
    return requestedType;
}

RetroFuturaGUI::PrimitiveUnion RetroFuturaGUI::IRangedValue::convertPrimitive(const PrimitiveUnion value, const PrimitiveTypeID from, const PrimitiveTypeID to)
{
    // Read through the member from says was written last, then let convertPrimitiveTo write through to's
    switch(from)
    {
        case PrimitiveTypeID::Int8:
            return convertPrimitiveTo(value.Int8, to);
        case PrimitiveTypeID::Int16:
            return convertPrimitiveTo(value.Int16, to);
        case PrimitiveTypeID::Int32:
            return convertPrimitiveTo(value.Int32, to);
        case PrimitiveTypeID::Int64:
            return convertPrimitiveTo(value.Int64, to);
        case PrimitiveTypeID::UInt8:
            return convertPrimitiveTo(value.UInt8, to);
        case PrimitiveTypeID::UInt16:
            return convertPrimitiveTo(value.UInt16, to);
        case PrimitiveTypeID::UInt32:
            return convertPrimitiveTo(value.UInt32, to);
        case PrimitiveTypeID::UInt64:
            return convertPrimitiveTo(value.UInt64, to);
        case PrimitiveTypeID::Float32:
            return convertPrimitiveTo(value.Float32, to);
        case PrimitiveTypeID::Float64:
            return convertPrimitiveTo(value.Float64, to);
        default: // Bool
            return convertPrimitiveTo(value.Bool, to);
    }
}

void RetroFuturaGUI::IRangedValue::setIndicatorPosition(const f32 trackFraction)
{
    const Background* track { getTrack() };

    if(!track)
        return;

    const f32
        borderInset { _border.GetWidth() * 2.0f },
        trackLength { glm::max(track->GetSize().x - borderInset, 0.0f) },
        indicatorLength { _indicator.GetDrawnSize().x },
        travelRange { 0.0f < trackLength - indicatorLength ? trackLength - indicatorLength : 0.0f },
        indicatorSliderPosition { indicatorLength * 0.5f + trackFraction * travelRange };

    // The indicator always travels along the track's local x-axis.
    // Rotate that local offset by the track's rotation to place it correctly in world space.
    const glm::vec2 localOffset(indicatorSliderPosition - trackLength * 0.5f, 0.0f);
    const f32 radians = glm::radians(track->GetRotation().z);
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    const glm::vec3 position
    (
        track->GetPosition().x + rotatedOffset.x,
        track->GetPosition().y + rotatedOffset.y,
        track->GetPosition().z + 0.02f
    );

    _indicator.Place(position, track->GetRotation());
}

void RetroFuturaGUI::IRangedValue::setGraphSize(const f32 trackFraction)
{
    const Background* track { getTrack() };

    if(!track)
        return;

    if(!_graph)
        return;

    const f32
        trackWidth { track->GetSize().x },
        trackHeight { track->GetSize().y },
        graphWidth { trackWidth * trackFraction },
        graphHeight { 0.0f < _graphWidth ? (_graphWidth < trackHeight ? _graphWidth : trackHeight) : trackHeight };

    _graph->SetSize(glm::vec2(graphWidth, graphHeight));
}

void RetroFuturaGUI::IRangedValue::setGraphPosition(const f32 trackFraction)
{
    const Background* track { getTrack() };

    if(!track)
        return;

    if(!_graph)
        return;

    setGraphSize(trackFraction);

    const f32
        trackWidth { track->GetSize().x },
        graphWidth { _graph->GetSize().x };

    // The graph always grows along the track's local x-axis, anchored to its left edge.
    const glm::vec2 localOffset(graphWidth * 0.5f - trackWidth * 0.5f, 0.0f);
    const f32 radians = glm::radians(track->GetRotation().z);
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    const glm::vec3 position
    (
        track->GetPosition().x + rotatedOffset.x,
        track->GetPosition().y + rotatedOffset.y,
        track->GetPosition().z + 0.02f
    );

    _graph->SetPosition(position);
    _graph->SetRotation(track->GetRotation());
}

void RetroFuturaGUI::IRangedValue::setIndicatorColors(const ColorState state)
{
    _indicator.SetColorState(state);
}

void RetroFuturaGUI::IRangedValue::setGraphColors(const ColorState state)
{
    _graphColorState = state;
    setGraphColorsApply();
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
    _indicator.Draw();
}

void RetroFuturaGUI::IRangedValue::drawGraph()
{
    if(!_useGraph || !_graph)
        return;

    _graph->Draw();
}

void RetroFuturaGUI::IRangedValue::setIndicatorSize()
{
    // The border shader insets the frame uBorderWidth inward from every edge, inside the track's own bounds,
    // so the area the indicator can occupy without painting over it is the track shrunk by that much per side.
    const glm::vec2 trackSize { _background.GetSize() };
    const f32 borderInset { _border.GetWidth() * 2.0f };
    const glm::vec2 innerSize { glm::max(trackSize.x - borderInset, 0.0f), glm::max(trackSize.y - borderInset, 0.0f) };

    //Both axes are the track's own - x along its length, y across its thickness - and the indicator carries the track's rotation.
    _indicator.Fit(innerSize);
}