#pragma once
#include "IWidget.hpp"
//#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace RetroFuturaGUI
{
    class Window : public IWidget
    {
    public:
        Window(const std::string& name, i32 width, i32 height, void* parent = nullptr, Sizing minWidth = Sizing::UseCurrent, Sizing minHeight = Sizing::UseCurrent, Sizing maxWidth = Sizing::UseCurrent, Sizing maxHeight = Sizing::UseCurrent);

        bool WindowShouldClose();
        void Draw();
        void SetWindowSize(i32 width, i32 height);
        void SetWidth(i32 width);
        void SetHeight(i32 height);
        void SetBackgroundColor(const glm::vec4& color);

    private:
        GLFWwindow* _window;
        glm::vec4 _backgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};



        static void mouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods);
        void renderCustomCloseButton();
        void renderCustomTopBar();
        void createWindow();
    };
}