#pragma once
#include <memory>
#include "Window.hpp"

namespace RetroFuturaGUI
{
    class MainWindow
    {
    public:
        MainWindow(const std::string& name, i32 width, i32 height, void* parent = nullptr);
        //MainWindow() = default;
        bool WindowShouldClose();
        void SetBackgroundColor(const glm::vec4& color);
        void Draw();
        ~MainWindow();

    private:
    std::unique_ptr<Window> _mainWindow;
    static inline bool _mainWindowInitialized = false;

    };
}