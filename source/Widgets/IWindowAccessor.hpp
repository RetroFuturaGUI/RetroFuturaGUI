#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    //An interface with widget properties needed to be implemented on several places.
    class IWindowAccessor
    {
    protected:
        GLFWwindow* _parentWindow { nullptr };
    };
}