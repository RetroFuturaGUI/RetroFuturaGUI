#pragma once
#include "config.hpp"

namespace RetroFuturaGUI
{
    /// @brief A cell addressed the way the containers store it. Two-dimensional containers disregard _Layer.
    struct TrackIndex
    {
        uSize
            _Row { 0 },
            _Column { 0 },
            _Layer { 0 };
    };

    /// @brief A cell addressed the way a world-space vector reads: x across, y down, z into the container.
    struct AxisIndex
    {
        uSize
            _X { 0 },
            _Y { 0 },
            _Z { 0 };

        /// @brief The one crossover between the two schemes: x is the column, y is the row.
        constexpr operator TrackIndex() const
        {
            return TrackIndex { ._Row = _Y, ._Column = _X, ._Layer = _Z };
        }
    };

    static_assert(static_cast<TrackIndex>(AxisIndex { ._X = 1, ._Y = 2, ._Z = 3 })._Column == 1, "AxisIndex x must map to the column");
    static_assert(static_cast<TrackIndex>(AxisIndex { ._X = 1, ._Y = 2, ._Z = 3 })._Row == 2, "AxisIndex y must map to the row");
    static_assert(static_cast<TrackIndex>(AxisIndex { ._X = 1, ._Y = 2, ._Z = 3 })._Layer == 3, "AxisIndex z must map to the layer");
}
