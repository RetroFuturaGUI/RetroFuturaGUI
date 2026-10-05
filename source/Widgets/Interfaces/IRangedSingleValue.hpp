#pragma once
#include "IRangedValue.hpp"
#include "Signal.hpp"

namespace RetroFuturaGUI
{
    /// @brief Base for widgets that show one value inside a min/max range (Slider, ProgressBar). Owns the value, keeps it inside the range and places the indicator and graph at it.
    class IRangedSingleValue : public IRangedValue
    {
    public:
        IRangedSingleValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        IRangedSingleValue() = delete;
        IRangedSingleValue(const IRangedSingleValue&) = delete;
        IRangedSingleValue(IRangedSingleValue&&) = delete;
        ~IRangedSingleValue() = default;
        auto operator =(const IRangedSingleValue&) = delete;
        auto operator =(IRangedSingleValue&&) = delete;

        /// @brief Connects a slot to be called when the value has changed
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnValueChanged(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the value has been set
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnValueSet(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnValueChanged slot.
        void Disconnect_OnValueChanged(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnValueSet slot.
        void Disconnect_OnValueSet(const typename Signal<>::Slot& slot);

        template <NumericValueType T> void SetValue(T value, const bool emitSignal = true)
        {
            if constexpr (std::is_same_v<T, i8>)
            {
                _value.Int8 = value < _minValue.Int8 ? _minValue.Int8 : value > _maxValue.Int8 ? _maxValue.Int8 : value;
                _valueType = PrimitiveTypeID::Int8;
            }
            else if constexpr (std::is_same_v<T, i16>)
            {
                _value.Int16 = value < _minValue.Int16 ? _minValue.Int16 : value > _maxValue.Int16 ? _maxValue.Int16 : value;
                _valueType = PrimitiveTypeID::Int16;
            }
            else if constexpr (std::is_same_v<T, i32>)
            {
                _value.Int32 = value < _minValue.Int32 ? _minValue.Int32 : value > _maxValue.Int32 ? _maxValue.Int32 : value;
                _valueType = PrimitiveTypeID::Int32;
            }
            else if constexpr (std::is_same_v<T, i64>)
            {
                _value.Int64 = value < _minValue.Int64 ? _minValue.Int64 : value > _maxValue.Int64 ? _maxValue.Int64 : value;
                _valueType = PrimitiveTypeID::Int64;
            }
            else if constexpr (std::is_same_v<T, u8>)
            {
                _value.UInt8 = value < _minValue.UInt8 ? _minValue.UInt8 : value > _maxValue.UInt8 ? _maxValue.UInt8 : value;
                _valueType = PrimitiveTypeID::UInt8;
            }
            else if constexpr (std::is_same_v<T, u16>)
            {
                _value.UInt16 = value < _minValue.UInt16 ? _minValue.UInt16 : value > _maxValue.UInt16 ? _maxValue.UInt16 : value;
                _valueType = PrimitiveTypeID::UInt16;
            }
            else if constexpr (std::is_same_v<T, u32>)
            {
                _value.UInt32 = value < _minValue.UInt32 ? _minValue.UInt32 : value > _maxValue.UInt32 ? _maxValue.UInt32 : value;
                _valueType = PrimitiveTypeID::UInt32;
            }
            else if constexpr (std::is_same_v<T, u64>)
            {
                _value.UInt64 = value < _minValue.UInt64 ? _minValue.UInt64 : value > _maxValue.UInt64 ? _maxValue.UInt64 : value;
                _valueType = PrimitiveTypeID::UInt64;
            }
            else if constexpr (std::is_same_v<T, f32>)
            {
                _value.Float32 = value < _minValue.Float32 ? _minValue.Float32 : value > _maxValue.Float32 ? _maxValue.Float32 : value;
                _valueType = PrimitiveTypeID::Float32;
            }
            else if constexpr (std::is_same_v<T, f64>)
            {
                _value.Float64 = value < _minValue.Float64 ? _minValue.Float64 : value > _maxValue.Float64 ? _maxValue.Float64 : value;
                _valueType = PrimitiveTypeID::Float64;
            }
            else
            {
                _value.Bool = value;
                _valueType = PrimitiveTypeID::Bool;
            }

            if(emitSignal)
            {
                _onValueChanged.Emit();
                _onValueChangedAsync.EmitAsync();
            }

            alignElementsToTrack();
        }


        template <NumericValueType T> const T GetValue() const
        {
            switch(_valueType)
            {
                case PrimitiveTypeID::Int8:
                {
                    if constexpr (std::is_same_v<T, i8>)
                        return _value.Int8;
                    else
                        return static_cast<T>(_value.Int8);
                }
                case PrimitiveTypeID::Int16:
                {
                    if constexpr (std::is_same_v<T, i16>)
                        return _value.Int16;
                    else
                        return static_cast<T>(_value.Int16);
                }
                case PrimitiveTypeID::Int32:
                {
                    if constexpr (std::is_same_v<T, i32>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.Int32);
                }
                case PrimitiveTypeID::Int64:
                {
                    if constexpr (std::is_same_v<T, i64>)
                        return _value.Int64;
                    else
                        return static_cast<T>(_value.Int64);
                }
                case PrimitiveTypeID::UInt8:
                {
                    if constexpr (std::is_same_v<T, u8>)
                        return _value.UInt8;
                    else
                        return static_cast<T>(_value.UInt8);
                }
                case PrimitiveTypeID::UInt16:
                {
                    if constexpr (std::is_same_v<T, u16>)
                        return _value.UInt16;
                    else
                        return static_cast<T>(_value.UInt16);
                }
                case PrimitiveTypeID::UInt32:
                {
                    if constexpr (std::is_same_v<T, u32>)
                        return _value.UInt32;
                    else
                        return static_cast<T>(_value.UInt32);
                }
                case PrimitiveTypeID::UInt64:
                {
                    if constexpr (std::is_same_v<T, u64>)
                        return _value.UInt64;
                    else
                        return static_cast<T>(_value.UInt64);
                }
                case PrimitiveTypeID::Float32:
                {
                    if constexpr (std::is_same_v<T, f32>)
                        return _value.Float32;
                    else
                        return static_cast<T>(_value.Float32);
                }
                case PrimitiveTypeID::Float64:
                {
                    if constexpr (std::is_same_v<T, f64>)
                        return _value.Float64;
                    else
                        return static_cast<T>(_value.Float64);
                }
                default:
                {
                    if constexpr (std::is_same_v<T, bool>)
                        return _value.Bool;
                    else
                         return static_cast<T>(_value.Bool);
                }
            }
        }

         /// @brief Sets the amount StepValue moves the value by.
        template <typename T> void SetStepSize(T value)
        {
            if constexpr (std::is_same_v<T, i8>)
                _stepSize.Int8 = value;
            else if constexpr (std::is_same_v<T, i16>)
                _stepSize.Int16 = value;
            else if constexpr (std::is_same_v<T, i32>)
                _stepSize.Int32 = value;
            else if constexpr (std::is_same_v<T, i64>)
                _stepSize.Int64 = value;
            else if constexpr (std::is_same_v<T, u8>)
                _stepSize.UInt8 = value;
            else if constexpr (std::is_same_v<T, u16>)
                _stepSize.UInt16 = value;
            else if constexpr (std::is_same_v<T, u32>)
                _stepSize.UInt32 = value;
            else if constexpr (std::is_same_v<T, u64>)
                _stepSize.UInt64 = value;
            else if constexpr (std::is_same_v<T, f32>)
                _stepSize.Float32 = value;
            else if constexpr (std::is_same_v<T, f64>)
                _stepSize.Float64 = value;
            else
                _stepSize.Bool = value;
        }

        
        /// @brief Moves the value one step toward the max (increase) or the min, clamped to the range.
        void StepValue(const bool increase);

    protected:
        /// @brief Clamps the value into the new range and emits OnValueChanged if that moved it.
        void alignValueToRange() override;

        /// @brief Places the indicator and graph at the value.
        void alignElementsToTrack() override;

        /// @brief Sets the value to where mousePos falls along the track - the inverse of how the indicator is placed.
        void setValueFromMousePosition(const glm::vec2& mousePos);

        Signal<>
            _onValueChanged,
            _onValueChangedAsync,
            _onValueSet,
            _onValueSetAsync;

    private:
        /// @brief Where the value sits in its range, 0 at the minimum. Independent of how the track is drawn.
        f32 getValueFraction() const;

        /// @brief Where along the track the value is drawn, 0 at the track's start. Positioning uses this; nothing else should.
        f32 getTrackFraction() const;

        /// @brief Clamps value into minValue..maxValue.
        /// @return Whether that changed the value.
        template <NumericValueType T> static bool clampToRange(T& value, const T minValue, const T maxValue)
        {
            const T clamped { value < minValue ? minValue : value > maxValue ? maxValue : value };

            if(clamped == value)
                return false;

            value = clamped;
            return true;
        }

        PrimitiveUnion _value { .UInt64 = 0 };
        PrimitiveUnion _stepSize { .UInt64 = 1 };
    };
}