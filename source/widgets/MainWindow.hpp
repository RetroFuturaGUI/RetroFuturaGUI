#pragma once
#include <memory>
#include "Window.hpp"

namespace RetroFuturaGUI
{
    class MainWindow
    {
    public:
        MainWindow(const std::string& name, i32 width, i32 height, void* parent = nullptr, Sizing minWidth = Sizing::UseCurrent, Sizing minHeight = Sizing::UseCurrent, Sizing maxWidth = Sizing::UseCurrent, Sizing maxHeight = Sizing::UseCurrent);
        //MainWindow() = default;
        bool WindowShouldClose();
        void Draw();
        ~MainWindow();

    private:
    std::unique_ptr<Window> _mainWindow;
    static inline bool _mainWindowInitialized = false;

    };
}