#pragma once
#include "IncludeHelper.hpp"
#include "InputManager.hpp"
#include "Label.hpp"
#include "WindowBar.hpp"

namespace RetroFuturaGUI
{
    class Window// : public IWidget
    {
    public:
        Window(const std::string& name, i32 width, i32 height, void* parent = nullptr);

        bool WindowShouldClose();
        void Draw();
        void SetWindowSize(i32 width, i32 height);
        void SetWidth(i32 width);
        void SetHeight(i32 height);
        void SetBackgroundColor(const glm::vec4& color);

    private:
        GLFWwindow* _window;
        u32 _width = 1280;
        u32 _height = 720;
        glm::vec4 _backgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};
        std::unique_ptr<Rectangle> _plane;
        std::unique_ptr<LineBorder> _lineBorder;
        std::unique_ptr<Projection> _projection;
        std::unique_ptr<Text> _text;
        std::unique_ptr<Label> _label;
        std::unique_ptr<Button> _button;
        std::unique_ptr<WindowBar> _windowBar;

        void createWindow();
        void setupProjectionMatrix();

        static void cursorPositionCallback(GLFWwindow* window, f64 xpos, f64 ypos)
        {
            InputManager::SetHoveredWindow(window);
            InputManager::SetMousePosition(xpos, ypos);
            // std::println("hover");
        }

        static void mouseButtonClickedCallback(GLFWwindow* window, i32 button, i32 action, i32 mods) 
        {
            if (action == GLFW_PRESS) 
            {
                InputManager::SetFocusedWindow(window);
               // std::println("mouse click: {}", button);
            }
            else if (action == GLFW_RELEASE) 
            {
                //std::println("mouse released: {}", button);
            }
            
            InputManager::SetMouseButtonState(button, action == GLFW_PRESS);
        }

        static void windowFocusCallback(GLFWwindow* window, i32 focused) 
        {
            if (focused)
            {
                InputManager::SetFocusedWindow(window);
                std::println("focused");
            } 
            else if (InputManager::GetFocusedWindow() == window)
            {
                InputManager::SetFocusedWindow(nullptr);
                std::println("unfocused");
            }
        }
    };
}