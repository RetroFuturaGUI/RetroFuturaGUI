#pragma once
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

    protected:
        /// @brief Nothing to clamp: the values belong to the caller and are mapped into the range when drawn.
        void alignValueToRange() override;

        /// @brief Nothing to place yet: multi-value widgets place the indicator and graph per value when they draw.
        void alignElementsToTrack() override;
    };
}
