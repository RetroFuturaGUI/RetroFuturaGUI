#include "IRangedMultiValue.hpp"

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

void RetroFuturaGUI::IRangedMultiValue::setData(void* data, const uSize count, const PrimitiveTypeID type)
{
    _data = data;
    _dataCount = count;
    setValueType(type);
    alignElementsToTrack();
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
    if(!_data)
        return requestedType;

    return _valueType;
}