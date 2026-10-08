#include "IRangedSingleValue.hpp"

RetroFuturaGUI::IRangedSingleValue::IRangedSingleValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IRangedValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
}

void RetroFuturaGUI::IRangedSingleValue::Connect_OnValueChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueChangedAsync.Connect(slot);
    else
        _onValueChanged.Connect(slot);
}

void RetroFuturaGUI::IRangedSingleValue::Connect_OnValueSet(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onValueSetAsync.Connect(slot);
    else
        _onValueSet.Connect(slot);
}

void RetroFuturaGUI::IRangedSingleValue::Disconnect_OnValueChanged(const typename Signal<>::Slot& slot)
{
    _onValueChanged.Disconnect(slot);
    _onValueChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::IRangedSingleValue::Disconnect_OnValueSet(const typename Signal<>::Slot& slot)
{
    _onValueSet.Disconnect(slot);
    _onValueSetAsync.Disconnect(slot);
}

void RetroFuturaGUI::IRangedSingleValue::StepValue(const bool increase)
{
    switch(_valueType)
    {
        case PrimitiveTypeID::Int8:
            SetValue<i8>(stepToward(_value.Int8, _stepSize.Int8, increase ? _maxValue.Int8 : _minValue.Int8));
        break;
        case PrimitiveTypeID::Int16:
            SetValue<i16>(stepToward(_value.Int16, _stepSize.Int16, increase ? _maxValue.Int16 : _minValue.Int16));
        break;
        case PrimitiveTypeID::Int32:
            SetValue<i32>(stepToward(_value.Int32, _stepSize.Int32, increase ? _maxValue.Int32 : _minValue.Int32));
        break;
        case PrimitiveTypeID::Int64:
            SetValue<i64>(stepToward(_value.Int64, _stepSize.Int64, increase ? _maxValue.Int64 : _minValue.Int64));
        break;
        case PrimitiveTypeID::UInt8:
            SetValue<u8>(stepToward(_value.UInt8, _stepSize.UInt8, increase ? _maxValue.UInt8 : _minValue.UInt8));
        break;
        case PrimitiveTypeID::UInt16:
            SetValue<u16>(stepToward(_value.UInt16, _stepSize.UInt16, increase ? _maxValue.UInt16 : _minValue.UInt16));
        break;
        case PrimitiveTypeID::UInt32:
            SetValue<u32>(stepToward(_value.UInt32, _stepSize.UInt32, increase ? _maxValue.UInt32 : _minValue.UInt32));
        break;
        case PrimitiveTypeID::UInt64:
            SetValue<u64>(stepToward(_value.UInt64, _stepSize.UInt64, increase ? _maxValue.UInt64 : _minValue.UInt64));
        break;
        case PrimitiveTypeID::Float32:
            SetValue<f32>(stepToward(_value.Float32, _stepSize.Float32, increase ? _maxValue.Float32 : _minValue.Float32));
        break;
        case PrimitiveTypeID::Float64:
            SetValue<f64>(stepToward(_value.Float64, _stepSize.Float64, increase ? _maxValue.Float64 : _minValue.Float64));
        break;
        default: // Bool
            SetValue<bool>(increase);
        break;
    }
}

void RetroFuturaGUI::IRangedSingleValue::convertValuesToType(const PrimitiveTypeID previousType)
{
    _value = convertPrimitive(_value, previousType, _valueType);
    _stepSize = convertPrimitive(_stepSize, previousType, _valueType);
}

void RetroFuturaGUI::IRangedSingleValue::alignValueToRange()
{
    bool isClamped { false };

    switch(_valueType)
    {
        case PrimitiveTypeID::Int8:
            isClamped = clampToRange(_value.Int8, _minValue.Int8, _maxValue.Int8);
        break;
        case PrimitiveTypeID::Int16:
            isClamped = clampToRange(_value.Int16, _minValue.Int16, _maxValue.Int16);
        break;
        case PrimitiveTypeID::Int32:
            isClamped = clampToRange(_value.Int32, _minValue.Int32, _maxValue.Int32);
        break;
        case PrimitiveTypeID::Int64:
            isClamped = clampToRange(_value.Int64, _minValue.Int64, _maxValue.Int64);
        break;
        case PrimitiveTypeID::UInt8:
            isClamped = clampToRange(_value.UInt8, _minValue.UInt8, _maxValue.UInt8);
        break;
        case PrimitiveTypeID::UInt16:
            isClamped = clampToRange(_value.UInt16, _minValue.UInt16, _maxValue.UInt16);
        break;
        case PrimitiveTypeID::UInt32:
            isClamped = clampToRange(_value.UInt32, _minValue.UInt32, _maxValue.UInt32);
        break;
        case PrimitiveTypeID::UInt64:
            isClamped = clampToRange(_value.UInt64, _minValue.UInt64, _maxValue.UInt64);
        break;
        case PrimitiveTypeID::Float32:
            isClamped = clampToRange(_value.Float32, _minValue.Float32, _maxValue.Float32);
        break;
        case PrimitiveTypeID::Float64:
            isClamped = clampToRange(_value.Float64, _minValue.Float64, _maxValue.Float64);
        break;
        default: // Bool: false..true holds every bool, so there is nothing to clamp
        break;
    }

    // Whoever mirrors the value (a table's scroll position, ...) has to hear that the new range moved it
    if(isClamped)
    {
        _onValueChanged.Emit();
        _onValueChangedAsync.EmitAsync();
    }

    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedSingleValue::alignElementsToTrack()
{
    const f32 trackFraction { getTrackFraction() };
    setIndicatorPosition(trackFraction);
    setGraphPosition(trackFraction);
}

void RetroFuturaGUI::IRangedSingleValue::setValueFromMousePosition(const glm::vec2& mousePos)
{
    Rectangle* track { getTrack() };

    if(!track)
        return;

    if(!_indicatorBackground)
        return;

    // Revert the track's rotation to bring the mouse position into the track's local space where it always runs along the local X axis
    const glm::vec2 translated { mousePos - glm::vec2(track->GetPosition().x, track->GetPosition().y) };
    const f32 radians { glm::radians(-track->GetRotation().z) };
    const glm::vec2 localMouse
    (
        translated.x * cos(radians) - translated.y * sin(radians),
        translated.x * sin(radians) + translated.y * cos(radians)
    );

    // Must match the span setIndicatorPosition places the indicator across, or the cursor drifts from it mid-drag.
    const f32
        borderInset { _border ? _border->GetBorderWidth() * 2.0f : 0.0f },
        trackLength { glm::max(track->GetSize().x - borderInset, 0.0f) },
        indicatorLength { _indicatorBackground->GetSize().x },
        travelRange { 0.0f < trackLength - indicatorLength ? trackLength - indicatorLength : 0.0f },
        trackNearEdge { -trackLength * 0.5f },
        mouseOffsetOnAxis { localMouse.x - trackNearEdge - indicatorLength * 0.5f },
        clampedOffset { 0.0f > mouseOffsetOnAxis ? 0.0f : (mouseOffsetOnAxis > travelRange ? travelRange : mouseOffsetOnAxis) },
        trackFraction { 0.0f != travelRange ? clampedOffset / travelRange : 0.0f },
        // Back from a position along the track to a position in the value's range - toTrackFraction is its own inverse
        fraction { toTrackFraction(trackFraction) };

    switch(_valueType)
    {
        case PrimitiveTypeID::Int8:
            _value.Int8 = interpolate(_minValue.Int8, _maxValue.Int8, fraction);
        break;
        case PrimitiveTypeID::Int16:
            _value.Int16 = interpolate(_minValue.Int16, _maxValue.Int16, fraction);
        break;
        case PrimitiveTypeID::Int32:
            _value.Int32 = interpolate(_minValue.Int32, _maxValue.Int32, fraction);
        break;
        case PrimitiveTypeID::Int64:
            _value.Int64 = interpolate(_minValue.Int64, _maxValue.Int64, fraction);
        break;
        case PrimitiveTypeID::UInt8:
            _value.UInt8 = interpolate(_minValue.UInt8, _maxValue.UInt8, fraction);
        break;
        case PrimitiveTypeID::UInt16:
            _value.UInt16 = interpolate(_minValue.UInt16, _maxValue.UInt16, fraction);
        break;
        case PrimitiveTypeID::UInt32:
            _value.UInt32 = interpolate(_minValue.UInt32, _maxValue.UInt32, fraction);
        break;
        case PrimitiveTypeID::UInt64:
            _value.UInt64 = interpolate(_minValue.UInt64, _maxValue.UInt64, fraction);
        break;
        case PrimitiveTypeID::Float32:
            _value.Float32 = interpolate(_minValue.Float32, _maxValue.Float32, fraction);
        break;
        case PrimitiveTypeID::Float64:
            _value.Float64 = interpolate(_minValue.Float64, _maxValue.Float64, fraction);
        break;
        default: // Bool
            _value.Bool = 0.5f <= fraction;
        break;
    }

    alignElementsToTrack();
}

f32 RetroFuturaGUI::IRangedSingleValue::getValueFraction() const
{
    return getRangeFraction(toF32(_value));
}

f32 RetroFuturaGUI::IRangedSingleValue::getTrackFraction() const
{
    return toTrackFraction(getValueFraction());
}