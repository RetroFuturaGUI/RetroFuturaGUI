#include "IRangedValue.hpp"
#include <memory>


RetroFuturaGUI::IRangedValue::IRangedValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _indicator.SetTransitionSettings(&_transitionSettings);
    _graph.SetTransitionSettings(&_transitionSettings);
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);
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

RetroFuturaGUI::Graph& RetroFuturaGUI::IRangedValue::GetGraph()
{
    return _graph;
}

const RetroFuturaGUI::Graph& RetroFuturaGUI::IRangedValue::GetGraph() const
{
    return _graph;
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

    const f32
        trackWidth { track->GetSize().x },
        trackHeight { track->GetSize().y },
        graphWidth { trackWidth * trackFraction },
        thickness { _graph.GetWidth() },
        graphHeight { 0.0f < thickness ? (thickness < trackHeight ? thickness : trackHeight) : trackHeight };

    _graph.SetSize(glm::vec2(graphWidth, graphHeight));
}

void RetroFuturaGUI::IRangedValue::setGraphPosition(const f32 trackFraction)
{
    const Background* track { getTrack() };

    if(!track)
        return;

    setGraphSize(trackFraction);

    const f32
        trackWidth { track->GetSize().x },
        graphWidth { _graph.GetSize().x };

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

    _graph.SetPosition(position);
    _graph.SetRotation(track->GetRotation());
}

void RetroFuturaGUI::IRangedValue::setIndicatorColors(const ColorState state)
{
    _indicator.SetColorState(state);
}

void RetroFuturaGUI::IRangedValue::setGraphColors(const ColorState state)
{
    _graph.SetColorState(state);
}

void RetroFuturaGUI::IRangedValue::drawIndicator()
{
    _indicator.Draw();
}

void RetroFuturaGUI::IRangedValue::drawGraph()
{
    _graph.Draw();
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