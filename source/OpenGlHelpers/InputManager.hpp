#pragma once
#include "IncludeHelper.hpp"

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

        static inline void SetMouseButtonState(int button, bool state) 
        {
            GetInstance()._mouseButtons[button] = state;
        }

        static inline void SetMousePosition(f64 x, f64 y) 
        {
            i32 width, height;

            if(_hoveredWindow)
                glfwGetWindowSize(_hoveredWindow, &width, &height);

                                                        //adjust to window logic
                                                        //      V
            GetInstance()._mousePosition = glm::dvec2(x, (f64)height - y);
        }

        static inline bool IsMouseButtonPressed(i32 button) 
        {
            return _mouseButtons[button];
        }

        static inline glm::dvec2 GetMousePosition() 
        {
            return GetInstance()._mousePosition;
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

            static inline GLFWwindow* _focusedWindow;
            static inline GLFWwindow* _hoveredWindow;
            static inline bool _mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1] = { false };
            glm::dvec2 _mousePosition = { 0.0, 0.0 };
    };
};