#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    class IWindowAccessor
    {
    protected:
        GLFWwindow* _parentWindow { nullptr };
    };
}