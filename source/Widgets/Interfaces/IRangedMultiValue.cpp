#include "IRangedMultiValue.hpp"

RetroFuturaGUI::IRangedMultiValue::IRangedMultiValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IRangedValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
}

void RetroFuturaGUI::IRangedMultiValue::alignValueToRange()
{
}

void RetroFuturaGUI::IRangedMultiValue::alignElementsToTrack()
{
}
