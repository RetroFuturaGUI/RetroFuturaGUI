#include "ITextTypes.hpp"

void RetroFuturaGUI::ITextTypes::ChangeType(const DataTypeID id)
{
    if(_currentDataType == id)
        return;

    switch(id)
    {
        case DataTypeID::Bool:
            _value.Bool = getValueAsT<bool>(_currentDataType);
        break;
        case DataTypeID::Int8:
            _value.Int8 = getValueAsT<i8>(_currentDataType);
        break;
        case DataTypeID::Int16:
            _value.Int16 = getValueAsT<i16>(_currentDataType);
        break;
        case DataTypeID::Int32:
            _value.Int32 = getValueAsT<i32>(_currentDataType);
        break;
        case DataTypeID::Int64:
            _value.Int64 = getValueAsT<i64>(_currentDataType);
        break;
        case DataTypeID::UInt8:
            _value.UInt8 = getValueAsT<u8>(_currentDataType);
        break;
        case DataTypeID::UInt16:
            _value.UInt16 = getValueAsT<u16>(_currentDataType);
        break;
        case DataTypeID::UInt32:
            _value.UInt32 = getValueAsT<u32>(_currentDataType);
        break;
        case DataTypeID::UInt64:
            _value.UInt64 = getValueAsT<u64>(_currentDataType);
        break;
        case DataTypeID::Float32:
            _value.Float32 = getValueAsT<f32>(_currentDataType);
        break;
        case DataTypeID::Float64:
            _value.Float64 = getValueAsT<f64>(_currentDataType);
        break;
        default: //Text keeps whatever _valueText already holds
            [[unlikely]]
        break;
    }

    _currentDataType = id;
    refreshValueText();
}

const std::string& RetroFuturaGUI::ITextTypes::GetValueText() const
{
    return _valueText;
}

void RetroFuturaGUI::ITextTypes::SetNumericBase(const u32 base)
{
    if(base > 1 && base < 37)
        _numericBase = base;
    else
        _numericBase = 10;

    refreshValueText();
}

void RetroFuturaGUI::ITextTypes::SetDecimalPrecision(const i32 precision)
{
    _precision = precision > 32 ? 32 : precision;
    refreshValueText();
}

void RetroFuturaGUI::ITextTypes::refreshValueText()
{
    switch(_currentDataType)
    {
        case DataTypeID::Text:
            break;
        case DataTypeID::Float32:
            _valueText = parseFloat(_value.Float32);
            break;
        case DataTypeID::Float64:
            _valueText = parseFloat(_value.Float64);
            break;
        default:
            _valueText = convertIntegralString();
            break;
    }
}

 std::string RetroFuturaGUI::ITextTypes::convertIntegralString() const
{
    switch(_currentDataType)
    {
        case DataTypeID::Bool:
            return _value.Bool == true ? "true" : "false";
        case DataTypeID::Int8:
            return parseIntegral(_value.Int8);
        case DataTypeID::Int16:
            return parseIntegral(_value.Int16);
        case DataTypeID::Int32:
            return parseIntegral(_value.Int32);
        case DataTypeID::Int64:
            return parseIntegral(_value.Int64);
        case DataTypeID::UInt8:
            return parseIntegral(_value.UInt8);
        case DataTypeID::UInt16:
            return parseIntegral(_value.UInt16);
        case DataTypeID::UInt32:
            return parseIntegral(_value.UInt32);
        default: //UInt64
            return parseIntegral(_value.UInt64);
    }
}