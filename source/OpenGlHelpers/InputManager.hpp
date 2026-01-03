#pragma once
#include "IncludeHelper.hpp"
#include <algorithm>
#include <ranges>

namespace RetroFuturaGUI
{
    class InputManager
    {
    public:
        static inline void Init()
        {
            _focusedWindow = nullptr;
            _hoveredWindow = nullptr;
        }

        static inline void SetFocusedWindow(GLFWwindow* window)
        {
            _focusedWindow = window;
        }

        static inline GLFWwindow* GetFocusedWindow()
        {
            return _focusedWindow;
        }

        static inline void SetHoveredWindow(GLFWwindow* window)
        {
            _hoveredWindow = window;
        }

        static inline GLFWwindow* GetHoveredWindow()
        {
            return _hoveredWindow;
        }

        static inline void SetMouseButtonState(i32 button, bool state) 
        {
            GetInstance()._mouseButtons[button] = state;
        }

        static inline void SetMousePositionInvertedY(f64 x, f64 y) 
        {
            i32 
                width { 0 }, 
                height { 0 };

            if(_hoveredWindow)
                glfwGetWindowSize(_hoveredWindow, &width, &height);

            GetInstance()._mousePositionInvertedY = glm::dvec2(x, (f64)height - y); //adjust Y to inverted coordinate system
        }

        static inline void SetMousePosition(f64 x, f64 y) 
        {
            i32 width, height;

            if(_hoveredWindow)
                glfwGetWindowSize(_hoveredWindow, &width, &height);

            GetInstance()._mousePosition = glm::dvec2(x, y);
        }

        static inline bool IsMouseButtonPressed(i32 button) 
        {
            return _mouseButtons[button];
        }

        static inline glm::dvec2 GetMousePositionInvertedY() 
        {
            return GetInstance()._mousePositionInvertedY;
        }

        static inline glm::dvec2 GetMousePosition() 
        {
            return GetInstance()._mousePosition;
        }

        static inline bool AnyMouseButtonPressed()
        {
            return std::ranges::find(_mouseButtons, _mouseButtons + 8, true)  != (_mouseButtons + 8);
        }

    private:
        InputManager() = default;
        ~InputManager() = default;
        InputManager(const InputManager&) = delete;
        InputManager(InputManager&&) = delete;
        void operator=(const InputManager&) = delete;
        void operator=(InputManager&&) = delete;
        static InputManager& GetInstance()
        {
            static InputManager Instance;
            return Instance;
        }

            static inline constinit const u32 _mouseButtonCount { GLFW_MOUSE_BUTTON_LAST + 1 };
            static inline GLFWwindow* _focusedWindow { nullptr };
            static inline GLFWwindow* _hoveredWindow { nullptr };
            static inline constinit bool _mouseButtons[_mouseButtonCount] { false };
            glm::dvec2 _mousePositionInvertedY { 0.0, 0.0 };
            glm::dvec2 _mousePosition { 0.0, 0.0 };
    };
};