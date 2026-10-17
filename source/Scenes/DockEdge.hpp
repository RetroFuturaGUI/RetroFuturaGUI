#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    /// @brief Identifies an edge: the one of a window's client area a Scene docks to, or the one of a Window another window docks to.
    enum class DockEdge : u32
    {
        Top = 0,
        Right = 1,
        Bottom = 2,
        Left = 3
    };
}
