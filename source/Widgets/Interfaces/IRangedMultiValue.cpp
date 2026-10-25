#include "IRangedMultiValue.hpp"
#include <algorithm>
#include <bit>
#include <format>
#include <string>
#include <type_traits>

RetroFuturaGUI::IRangedMultiValue::IRangedMultiValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IRangedValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
}

void RetroFuturaGUI::IRangedMultiValue::Connect_OnDataSet(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onDataSetAsync.Connect(slot);
    else
        _onDataSet.Connect(slot);
}

void RetroFuturaGUI::IRangedMultiValue::Disconnect_OnDataSet(const typename Signal<>::Slot& slot)
{
    _onDataSet.Disconnect(slot);
    _onDataSetAsync.Disconnect(slot);
}

void RetroFuturaGUI::IRangedMultiValue::EnableClampToLastValue(const bool enable)
{
    _clampToLastValue = enable;
}

void RetroFuturaGUI::IRangedMultiValue::ShowHexNumbers(const bool show)
{
    _hexNumbers = show;
}

bool RetroFuturaGUI::IRangedMultiValue::setData(const uSize nthSet, const void* data, const uSize count, const PrimitiveTypeID type)
{
    // Every set is read as _valueType, so the type may only change while no other set holds values. Unbinding brings no values, so it is always allowed.
    if(0 != count && type != _valueType && isOtherSetBound(nthSet))
        return false;

    if(_data.size() < nthSet + 1)
        _data.resize(nthSet + 1);

    if(_dataCount.size() < nthSet + 1)
        _dataCount.resize(nthSet + 1);

    _data[nthSet] = data;
    _dataCount[nthSet] = count;
    _maxDataCount = (std::ranges::max)(_dataCount);

    if(0 != count)
        setValueType(type);

    alignElementsToTrack();
    return true;
}

f32 RetroFuturaGUI::IRangedMultiValue::getDataValueAsF32(const uSize nthSet, const uSize index) const
{
    switch(_valueType)
    {
        case PrimitiveTypeID::Bool:
            return getDataValue<bool>(nthSet, index) ? 1.0f : 0.0f;
        case PrimitiveTypeID::Int8:
            return static_cast<f32>(getDataValue<i8>(nthSet, index));
        case PrimitiveTypeID::Int16:
            return static_cast<f32>(getDataValue<i16>(nthSet, index));
        case PrimitiveTypeID::Int32:
            return static_cast<f32>(getDataValue<i32>(nthSet, index));
        case PrimitiveTypeID::Int64:
            return static_cast<f32>(getDataValue<i64>(nthSet, index));
        case PrimitiveTypeID::UInt8:
            return static_cast<f32>(getDataValue<u8>(nthSet, index));
        case PrimitiveTypeID::UInt16:
            return static_cast<f32>(getDataValue<u16>(nthSet, index));
        case PrimitiveTypeID::UInt32:
            return static_cast<f32>(getDataValue<u32>(nthSet, index));
        case PrimitiveTypeID::UInt64:
            return static_cast<f32>(getDataValue<u64>(nthSet, index));
        case PrimitiveTypeID::Float64:
            return saturatingCast<f32>(getDataValue<f64>(nthSet, index)); // beyond f32's range a plain cast is undefined behavior
        default: // Float32
            return getDataValue<f32>(nthSet, index);
    }
}

std::string RetroFuturaGUI::IRangedMultiValue::getDataValueAsString(const uSize nthSet, const uSize index) const
{
    switch(_valueType)
    {
        case PrimitiveTypeID::Bool:
            return formatValue(getDataValue<bool>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::Int8:
            return formatValue(getDataValue<i8>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::Int16:
            return formatValue(getDataValue<i16>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::Int32:
            return formatValue(getDataValue<i32>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::Int64:
            return formatValue(getDataValue<i64>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::UInt8:
            return formatValue(getDataValue<u8>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::UInt16:
            return formatValue(getDataValue<u16>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::UInt32:
            return formatValue(getDataValue<u32>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::UInt64:
            return formatValue(getDataValue<u64>(nthSet, index), _hexNumbers);
        case PrimitiveTypeID::Float64:
            return formatValue(getDataValue<f64>(nthSet, index), _hexNumbers);
        default: // Float32
            return formatValue(getDataValue<f32>(nthSet, index), _hexNumbers);
    }
}

bool RetroFuturaGUI::IRangedMultiValue::isOtherSetBound(const uSize nthSet) const
{
    for(uSize i = 0; i < _dataCount.size(); ++i)
    {
        if(i != nthSet && 0 != _dataCount[i])
            return true;
    }

    return false;
}

void RetroFuturaGUI::IRangedMultiValue::alignValueToRange()
{
}

void RetroFuturaGUI::IRangedMultiValue::alignElementsToTrack()
{
}

void RetroFuturaGUI::IRangedMultiValue::convertValuesToType(const PrimitiveTypeID)
{
}

RetroFuturaGUI::PrimitiveTypeID RetroFuturaGUI::IRangedMultiValue::resolveValueType(const PrimitiveTypeID requestedType) const
{
    if(0 == _maxDataCount)
        return requestedType;

    return _valueType;
}