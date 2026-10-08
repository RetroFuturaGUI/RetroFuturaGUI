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

        /// @brief Sets the value, clamped to the range. A T different from the range's first converts the range and the step to T (see setValueType).
        template <NumericValueType T> void SetValue(T value, const bool emitSignal = true)
        {
            setValueType(GetPrimitiveTypeID<T>());

            if constexpr (std::is_same_v<T, i8>)
                _value.Int8 = value < _minValue.Int8 ? _minValue.Int8 : value > _maxValue.Int8 ? _maxValue.Int8 : value;
            else if constexpr (std::is_same_v<T, i16>)
                _value.Int16 = value < _minValue.Int16 ? _minValue.Int16 : value > _maxValue.Int16 ? _maxValue.Int16 : value;
            else if constexpr (std::is_same_v<T, i32>)
                _value.Int32 = value < _minValue.Int32 ? _minValue.Int32 : value > _maxValue.Int32 ? _maxValue.Int32 : value;
            else if constexpr (std::is_same_v<T, i64>)
                _value.Int64 = value < _minValue.Int64 ? _minValue.Int64 : value > _maxValue.Int64 ? _maxValue.Int64 : value;
            else if constexpr (std::is_same_v<T, u8>)
                _value.UInt8 = value < _minValue.UInt8 ? _minValue.UInt8 : value > _maxValue.UInt8 ? _maxValue.UInt8 : value;
            else if constexpr (std::is_same_v<T, u16>)
                _value.UInt16 = value < _minValue.UInt16 ? _minValue.UInt16 : value > _maxValue.UInt16 ? _maxValue.UInt16 : value;
            else if constexpr (std::is_same_v<T, u32>)
                _value.UInt32 = value < _minValue.UInt32 ? _minValue.UInt32 : value > _maxValue.UInt32 ? _maxValue.UInt32 : value;
            else if constexpr (std::is_same_v<T, u64>)
                _value.UInt64 = value < _minValue.UInt64 ? _minValue.UInt64 : value > _maxValue.UInt64 ? _maxValue.UInt64 : value;
            else if constexpr (std::is_same_v<T, f32>)
                _value.Float32 = value < _minValue.Float32 ? _minValue.Float32 : value > _maxValue.Float32 ? _maxValue.Float32 : value;
            else if constexpr (std::is_same_v<T, f64>)
                _value.Float64 = value < _minValue.Float64 ? _minValue.Float64 : value > _maxValue.Float64 ? _maxValue.Float64 : value;
            else
                _value.Bool = value;

            if(emitSignal)
            {
                _onValueChanged.Emit();
                _onValueChangedAsync.EmitAsync();
            }

            alignElementsToTrack();
        }


        /// @brief Returns the value converted to T. Saturates where T can't hold it (a negative value as unsigned gives 0) instead of the undefined behavior of a plain cast.
        template <NumericValueType T> const T GetValue() const
        {
            switch(_valueType)
            {
                case PrimitiveTypeID::Int8:
                    return saturatingCast<T>(_value.Int8);
                case PrimitiveTypeID::Int16:
                    return saturatingCast<T>(_value.Int16);
                case PrimitiveTypeID::Int32:
                    return saturatingCast<T>(_value.Int32);
                case PrimitiveTypeID::Int64:
                    return saturatingCast<T>(_value.Int64);
                case PrimitiveTypeID::UInt8:
                    return saturatingCast<T>(_value.UInt8);
                case PrimitiveTypeID::UInt16:
                    return saturatingCast<T>(_value.UInt16);
                case PrimitiveTypeID::UInt32:
                    return saturatingCast<T>(_value.UInt32);
                case PrimitiveTypeID::UInt64:
                    return saturatingCast<T>(_value.UInt64);
                case PrimitiveTypeID::Float32:
                    return saturatingCast<T>(_value.Float32);
                case PrimitiveTypeID::Float64:
                    return saturatingCast<T>(_value.Float64);
                default: // Bool
                    return saturatingCast<T>(_value.Bool);
            }
        }

        /// @brief Sets the amount StepValue moves the value by - zero or positive, in the T the range was set with (see SetValue).
        template <NumericValueType T> void SetStepSize(T value)
        {
            setValueType(GetPrimitiveTypeID<T>());

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

        /// @brief Converts the value and the step from previousType to the new type. A fractional step becomes 0 as an integer type, so StepValue then stays put until SetStepSize.
        void convertValuesToType(const PrimitiveTypeID previousType) override;

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

        /// @brief value moved one step toward bound, stopping at bound.
        /// @note Integer distances are taken in the unsigned type, where they can't overflow however wide the range is.
        template <NumericValueType T> requires (!std::same_as<T, bool>) static T stepToward(const T value, const T step, const T bound)
        {
            const bool isUpward { value < bound };

            if constexpr (std::is_integral_v<T>)
            {
                using UnsignedT = std::make_unsigned_t<T>;

                const UnsignedT distance { static_cast<UnsignedT>(isUpward
                    ? static_cast<UnsignedT>(bound) - static_cast<UnsignedT>(value)
                    : static_cast<UnsignedT>(value) - static_cast<UnsignedT>(bound)) };

                if(distance <= static_cast<UnsignedT>(step))
                    return bound;

                // distance > step, so this stays between value and bound
                return static_cast<T>(isUpward ? value + step : value - step);
            }
            else
            {
                const T distance { isUpward ? bound - value : value - bound };

                if(distance <= step)
                    return bound;

                return isUpward ? value + step : value - step;
            }
        }

        /// @brief The point fraction (0..1) of the way from minValue to maxValue.
        /// @note Computed in f64, where an integer range can't overflow, then saturated into T and clamped to the range against rounding.
        template <NumericValueType T> requires (!std::same_as<T, bool>) static T interpolate(const T minValue, const T maxValue, const f32 fraction)
        {
            const f64 interpolated { static_cast<f64>(minValue) + static_cast<f64>(fraction) * (static_cast<f64>(maxValue) - static_cast<f64>(minValue)) };
            const T value { saturatingCast<T>(interpolated) };
            return value < minValue ? minValue : value > maxValue ? maxValue : value;
        }

        PrimitiveUnion
            _value { .Int32 = 0 },
            _stepSize { .Int32 = 1 };
    };
}