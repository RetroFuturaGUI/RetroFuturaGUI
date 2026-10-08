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

         /// @brief Disconnects a previously connected OnValueSet slot.
        void Disconnect_OnDataSet(const typename Signal<>::Slot& slot);

        template<NumericValueType T> void SetData(std::span<T> data, const bool emitSignal)
        {    
            setData(data.data(), data.size(), GetPrimitiveTypeID<T>());
            _stride = sizeof(T);

            if(emitSignal)
            {
                _onDataSet.Emit();
                _onDataSetAsync.EmitAsync();
            }
        }

    protected:
        /// @brief Nothing to clamp: the values belong to the caller and are mapped into the range when drawn.
        void alignValueToRange() override;

        /// @brief Nothing to place yet: multi-value widgets place the indicator and graph per value when they draw.
        void alignElementsToTrack() override;

        /// @brief Nothing to convert: the data belongs to the caller and brings its own type.
        void convertValuesToType(const PrimitiveTypeID previousType) override;

        /// @brief The data's type once there is data - the buffer can't change type, so a bound set in another type is converted to the data's instead.
        /// Without data, requestedType; the next SetData then converts the range to the data's type.
        PrimitiveTypeID resolveValueType(const PrimitiveTypeID requestedType) const override;

        void setData(void* data, const uSize count, const PrimitiveTypeID type);
        
        template<NumericValueType T> T getDataValue(const uSize index) const
        {
            if(!_data)
                return T{};

            if(index >= _dataCount)
                return T{};

            return static_cast<T*>(_data)[index];
        }

        void* _data { nullptr };
        uSize
            _dataCount { 0 },
            _maxDataCount { 0 },
            _stride { 4 };
        bool _clampToLastValue { false };

        Signal<>
            _onDataSet,
            _onDataSetAsync;

    private:


    };
}
