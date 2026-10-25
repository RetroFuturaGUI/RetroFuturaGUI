#pragma once
#include "Signal.hpp"
#include "IRangedValue.hpp"

namespace RetroFuturaGUI
{
    /// @brief Base for widgets that show many values inside one min/max range (Histogram, LineGraph). The values belong to the caller, so nothing is clamped - a value outside the range simply maps outside 0..1.
    class IRangedMultiValue : public IRangedValue
    {
    public:
        IRangedMultiValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        IRangedMultiValue() = delete;
        IRangedMultiValue(const IRangedMultiValue&) = delete;
        IRangedMultiValue(IRangedMultiValue&&) = delete;
        ~IRangedMultiValue() = default;
        auto operator =(const IRangedMultiValue&) = delete;
        auto operator =(IRangedMultiValue&&) = delete;

        /// @brief Connects a slot to be called when the value has been set
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnDataSet(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnDataSet slot.
        void Disconnect_OnDataSet(const typename Signal<>::Slot& slot);

        /// @brief Shows data without copying it - it is read every frame, so it must stay valid, and its buffer must not move, until the next SetData.
        /// All sets are read in one type, so a set may only bring a type of its own while no other set holds values. An empty span unbinds the set.
        /// @return false when another set holds values of a different type, in which case nothing is changed.
        template<NumericValueType T> bool SetData(const uSize nthSet, std::span<const T> data, const bool emitSignal)
        {
            if(!setData(nthSet, data.data(), data.size(), GetPrimitiveTypeID<T>()))
                return false;

            if(emitSignal)
            {
                _onDataSet.Emit();
                _onDataSetAsync.EmitAsync();
            }

            return true;
        }

        /// @brief Sets whether a set shorter than the longest holds its last value up to the end of the track, instead of ending where its data does.
        void SetClampToLastValue(const bool value);

        /// @brief Sets whether values are shown in hexadecimal: an integer as its value (17 is 0x11, -1 in an i8 is 0xFF),
        /// a float as its bits in full (1.0f is 0x3F800000, 1.0 is 0x3FF0000000000000).
        void HexNumbers(const bool value);

    protected:
        /// @brief Nothing to clamp: the values belong to the caller and are mapped into the range when drawn.
        void alignValueToRange() override;

        /// @brief Nothing to place yet: multi-value widgets place the indicator and graph per value when they draw.
        void alignElementsToTrack() override;

        /// @brief Nothing to convert: the data belongs to the caller and brings its own type.
        void convertValuesToType(const PrimitiveTypeID previousType) override;

        /// @brief The data's type once any set holds values - the buffers can't change type, so a bound set in another type is converted to the data's instead.
        /// Without data, requestedType; the next SetData then converts the range to the data's type.
        PrimitiveTypeID resolveValueType(const PrimitiveTypeID requestedType) const override;

        /// @return false when another set holds values of a different type, in which case nothing is changed. See SetData.
        bool setData(const uSize nthSet, const void* data, const uSize count, const PrimitiveTypeID type);

        /// @brief The value at index of the nthSet, read as _valueType and converted to f32. 0 for a set or index that holds no value.
        f32 getDataValueAsF32(const uSize nthSet, const uSize index) const;

        /// @brief The value at index of the nthSet as text, in its own type, so nothing is lost converting it - decimal floats as short as still reads back exactly,
        /// or hexadecimal while HexNumbers is on.
        std::string getDataValueAsString(const uSize nthSet, const uSize index) const;

        /// @note T must be _valueType, or the buffer is read as the wrong type - getDataValueAsF32 picks it for you.
        template<NumericValueType T> T getDataValue(const uSize nthSet, const uSize index) const
        {
            if(nthSet >= _data.size() || nthSet >= _dataCount.size())
                return T{};

            if(!_data[nthSet])
                return T{};

            if(index >= _dataCount[nthSet])
                return T{};

            return static_cast<const T*>(_data[nthSet])[index];
        }

        std::vector<const void*> _data {};
        std::vector<uSize> _dataCount {};
        uSize _maxDataCount { 0 }; // the longest set's count; 0 while no set holds values
        bool
            _clampToLastValue { false },
            _hexNumbers { false };

        Signal<>
            _onDataSet,
            _onDataSetAsync;

    private:
        /// @brief Whether a set other than nthSet holds values - those are read as _valueType, so the type can't change while one does.
        bool isOtherSetBound(const uSize nthSet) const;

        template<RetroFuturaGUI::NumericValueType T> static std::string formatValue(const T value, const bool hexNumbers)
        {
            if constexpr (std::same_as<T, bool>)
                return hexNumbers ? (value ? "0x1" : "0x0") : (value ? "true" : "false");
            else if constexpr (std::is_floating_point_v<T>)
            {
                if(!hexNumbers)
                    return std::format("{}", value);

                using Bits = std::conditional_t<sizeof(T) == sizeof(u32), u32, u64>;
                return std::format("0x{:0{}X}", std::bit_cast<Bits>(value), sizeof(T) * 2);
            }
            else
            {
                if(!hexNumbers)
                    return std::format("{}", value);

                return std::format("0x{:X}", static_cast<std::make_unsigned_t<T>>(value));
            }
        }
    };
}
