#pragma once
#include "IncludeHelper.hpp"
#include <print>

namespace RetroFuturaGUI
{
    static inline constinit bool GlfwInitialized { false };

    static i32 GlfwInit()
    {
        if(GlfwInitialized)
        {
            std::println("ERROR: glfw already initialized");
            return -1;
        }

        if (!glfwInit())
        {
            std::println("ERROR: glfw couldn't start");
            return -1;
        }

        GlfwInitialized = true;
        return 0;
    }

    static void GlfwTerminate()
    {
        glfwTerminate();
    }
}