#pragma once
#include "IncludeHelper.hpp"
#include <concepts>
#include <type_traits>
#include <string_view>
#include <string>
#include <charconv>
#include <array>
#include <bit>
#include <format>
#include <optional>

namespace RetroFuturaGUI
{
    /// @brief The value types ITextTypes can hold: exactly the members of ITextTypes::ValueType, plus text.
    template<typename T>
    concept TextValueType =
        std::same_as<T, bool> ||
        std::same_as<T, i8>  || std::same_as<T, i16> || std::same_as<T, i32> || std::same_as<T, i64> ||
        std::same_as<T, u8>  || std::same_as<T, u16> || std::same_as<T, u32> || std::same_as<T, u64> ||
        std::same_as<T, f32> || std::same_as<T, f64> ||
        std::same_as<T, std::string_view>;

    template<typename T>
    concept NumericValueType =
        std::same_as<T, bool> ||
        std::same_as<T, i8>  || std::same_as<T, i16> || std::same_as<T, i32> || std::same_as<T, i64> ||
        std::same_as<T, u8>  || std::same_as<T, u16> || std::same_as<T, u32> || std::same_as<T, u64> ||
        std::same_as<T, f32> || std::same_as<T, f64>;

    class ITextTypes
    {
    public:
        union ValueType
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
        } _value { .UInt64 = 0 };

        enum class DataTypeID : u32
        {
            Bool,
            Int8,
            Int16,
            Int32,
            Int64,
            UInt8,
            UInt16,
            UInt32,
            UInt64,
            Float32,
            Float64,
            Text
        };

        void ChangeType(const DataTypeID id);

        /// @brief Returns the stored value rendered as text in the current numeric base.
        const std::string& GetValueText() const;

        /// @brief Returns the stored value converted to T. Parses _valueText when the current type is Text.
        template<NumericValueType T>
        T GetValue() const
        {
            return getValueAsT<T>(_currentDataType);
        }

        template<TextValueType T>
        void SetValue(const T value)
        {
            if constexpr (std::same_as<T, std::string_view>)
            {
                _currentDataType = DataTypeID::Text;
                _valueText = value;
            }
            else if constexpr (std::same_as<T, bool>)
            {
                _currentDataType = DataTypeID::Bool;
                _value.Bool = value;
            }
            else if constexpr (std::same_as<T, i8>)
            {
                _currentDataType = DataTypeID::Int8;
                _value.Int8 = value;
            }
            else if constexpr (std::same_as<T, i16>)
            {
                _currentDataType = DataTypeID::Int16;
                _value.Int16 = value;
            }
            else if constexpr (std::same_as<T, i32>)
            {
                _currentDataType = DataTypeID::Int32;
                _value.Int32 = value;
            }
            else if constexpr (std::same_as<T, i64>)
            {
                _currentDataType = DataTypeID::Int64;
                _value.Int64 = value;
            }
            else if constexpr (std::same_as<T, u8>)
            {
                _currentDataType = DataTypeID::UInt8;
                _value.UInt8 = value;
            }
            else if constexpr (std::same_as<T, u16>)
            {
                _currentDataType = DataTypeID::UInt16;
                _value.UInt16 = value;
            }
            else if constexpr (std::same_as<T, u32>)
            {
                _currentDataType = DataTypeID::UInt32;
                _value.UInt32 = value;
            }
            else if constexpr (std::same_as<T, u64>)
            {
                _currentDataType = DataTypeID::UInt64;
                _value.UInt64 = value;
            }
            else if constexpr (std::same_as<T, f32>)
            {
                _currentDataType = DataTypeID::Float32;
                _value.Float32 = value;
            }
            else // f64
            {
                _currentDataType = DataTypeID::Float64;
                _value.Float64 = value;
            }

            refreshValueText();
        }

        void SetNumericBase(const u32 base);
        void SetDecimalPrecision(const i32 precision);

    protected:
        /// @brief Re-renders the stored value into _valueText using the current numeric base. A no-op for DataTypeID::Text.
        void refreshValueText();

        u32 _numericBase { 10 };
        DataTypeID _currentDataType { DataTypeID::Text };
        std::string _valueText {};

    private:
        i32 _precision { 4 };

        template<NumericValueType T>
        T getValueAsT(const DataTypeID id) const
        {
            switch(id)
            {
                case DataTypeID::Bool:
                    return static_cast<T>(_value.Bool);
                case DataTypeID::Int8:
                    return static_cast<T>(_value.Int8);
                case DataTypeID::Int16:
                    return static_cast<T>(_value.Int16);
                case DataTypeID::Int32:
                    return static_cast<T>(_value.Int32);
                case DataTypeID::Int64:
                    return static_cast<T>(_value.Int64);
                case DataTypeID::UInt8:
                    return static_cast<T>(_value.UInt8);
                case DataTypeID::UInt16:
                    return static_cast<T>(_value.UInt16);
                case DataTypeID::UInt32:
                    return static_cast<T>(_value.UInt32);
                case DataTypeID::UInt64:
                    return static_cast<T>(_value.UInt64);
                case DataTypeID::Float32:
                    return static_cast<T>(_value.Float32);
                case DataTypeID::Float64:
                    return static_cast<T>(_value.Float64);
                case DataTypeID::Text:
                    return parseValueText<T>().value_or(T {});
            }

            return T {}; //id is u32-backed, so a value outside the enumerators is representable
        }

        /// @brief Parses _valueText back into T, inverting parseIntegral/parseFloat.
        /// @return nullopt if the text is not a valid representation in the current base.
        template<NumericValueType T>
        std::optional<T> parseValueText() const
        {
            const char* const begin = _valueText.data();
            const char* const end = begin + _valueText.size();

            if constexpr (std::same_as<T, bool>)
            {
                if(_valueText == "true")
                    return true;

                if(_valueText == "false")
                    return false;

                u64 raw {};
                const std::from_chars_result result = std::from_chars(begin, end, raw, static_cast<i32>(_numericBase));

                if(result.ec != std::errc {} || result.ptr != end)
                    return false;

                return raw != 0;
            }
            else if constexpr (std::floating_point<T>)
            {
                if(_numericBase == 2 || _numericBase == 16)
                {
                    std::string digits;
                    digits.reserve(_valueText.size());

                    for(const char c : _valueText)
                        if(c != '.')
                            digits.push_back(c);

                    std::conditional_t<std::same_as<T, f64>, u64, u32> bits {};
                    const std::from_chars_result result =
                        std::from_chars(digits.data(), digits.data() + digits.size(),
                            bits, static_cast<i32>(_numericBase));

                    if(result.ec != std::errc {} || result.ptr != digits.data() + digits.size())
                        return std::nullopt;

                    return std::bit_cast<T>(bits);
                }

                T value {};
                const std::from_chars_result result = std::from_chars(begin, end, value);

                if(result.ec != std::errc {} || result.ptr != end)
                    return std::nullopt;

                return value;
            }
            else
            {
                T value {};
                const std::from_chars_result result = std::from_chars(begin, end, value, static_cast<i32>(_numericBase));

                if(result.ec != std::errc {} || result.ptr != end)
                    return std::nullopt;

                return value;
            }
        }

        std::string convertIntegralString() const;

        template<std::integral T> std::string parseIntegral(const T value) const
        {
            std::array<char, 80> buffer {};
            const std::to_chars_result result =
                std::to_chars(buffer.data(), buffer.data() + buffer.size(),
                    value, static_cast<i32>(_numericBase));

            return std::string(buffer.data(), result.ptr);
        }

        /// @brief Renders a float. Base 2 and 16 show the IEEE-754 bit pattern, every other base shows decimal.
        template<std::floating_point T> std::string parseFloat(const T value) const
        {
            if(_numericBase == 2 || _numericBase == 16)
            {
                if constexpr (std::same_as<T, f64>)
                {
                    const u64 bits = std::bit_cast<u64>(value);

                    if(_numericBase == 16)
                        return std::format("{:016x}", bits);

                    const std::string str = std::format("{:064b}", bits);
                    return str.substr(0, 12) + "." + str.substr(12, 52);
                }
                else
                {
                    const u32 bits = std::bit_cast<u32>(value);

                    if(_numericBase == 16)
                        return std::format("{:08x}", bits);

                    const std::string str = std::format("{:032b}", bits);
                    return str.substr(0, 9) + "." + str.substr(9, 23);
                }
            }

            if(_precision < 0) //shortest representation that still round-trips
            {
                std::array<char, 80> buffer {};
                const std::to_chars_result result =
                    std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);

                return std::string(buffer.data(), result.ptr);
            }

            return std::format("{:.{}f}", value, _precision);
        }
    };
}