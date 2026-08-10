#pragma once
#include "IncludeHelper.hpp"
#include <print>

namespace RetroFuturaGUI
{
    static inline constinit bool GlfwInitialized { false };

    /// @brief Initializes GLFW. Safe to call only once per process.
    /// @return 0 on success, or -1 if GLFW was already initialized or failed to start.
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

    /// @brief Terminates GLFW, releasing all windows and resources it owns.
    static void GlfwTerminate()
    {
        glfwTerminate();
    }
}