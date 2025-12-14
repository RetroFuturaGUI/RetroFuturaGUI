#pragma once
#include "IncludeHelper.hpp"
#include "InputManager.hpp"
#include "Label.hpp"
#include "WindowBar.hpp"
#include "Image2D.hpp"
#include "Grid2D.hpp"
#include <algorithm>

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
        void createWindow();
        static void cursorPositionCallback(GLFWwindow* window, f64 xpos, f64 ypos);
        void setCursorPosition();
        void setCursorIcon();
        static void mouseButtonClickedCallback(GLFWwindow* window, i32 button, i32 action, i32 mods);
        void setResizeState(i32 button, i32 action, i32 mods);
        void resize();
        static void windowFocusCallback(GLFWwindow* window, i32 focused);

        enum class ResizeEdge : u32
        {
            NONE,
            LEFT,
            RIGHT,
            TOP,
            BOTTOM,
            TOP_LEFT,
            TOP_RIGHT,
            BOTTOM_LEFT,
            BOTTOM_RIGHT
        };

        // Window properties
        GLFWwindow* _window;
        i32 
            _width { 1280 },
            _height { 720 },
            _minWindowSpan { 100 };
        f64 
            _prevResizeX { 0.0 },
            _prevResizeY { 0.0 },
            _cursorPosX { 0.0 },
            _cursorPosY { 0.0 },
            _boundaryThreshold { 10.0 };
        ResizeEdge _resizeEdge { ResizeEdge::NONE };
        bool _isResizing { false };
        glm::vec4 _backgroundColor { 0.1f, 0.1f, 0.1f, 1.0f };
        std::unique_ptr<Projection> _projection;

        // todo: create a cursor manager once the TextBox widget exists
        static inline GLFWcursor* _resizeCursorHorizontal { nullptr };
        static inline GLFWcursor* _resizeCursorVertical { nullptr };
        static inline GLFWcursor* _resizeCursorTLBR { nullptr };
        static inline GLFWcursor* _resizeCursorTRBL { nullptr };
        static inline GLFWcursor* _defaultCursor { nullptr };
        static inline GLFWcursor* _cursorIcon { nullptr };
        bool _cursorsInitialized { false };

        // test widgets
        std::unique_ptr<Rectangle> _plane;
        std::unique_ptr<LineBorder> _lineBorder;
        std::unique_ptr<Text> _text;
        std::unique_ptr<Label> _label;
        std::unique_ptr<Button> _button;
        std::unique_ptr<WindowBar> _windowBar;
        std::unique_ptr<Image2D> _texture;
        std::unique_ptr<Image2D> _backgroundImage;
        std::unique_ptr<Grid2d> _grid;
    };
}