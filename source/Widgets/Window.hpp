#pragma once
#include "IncludeHelper.hpp"
#include "InputManager.hpp"
#include "Label.hpp"
#include "WindowBar.hpp"
#include "Image.hpp"
#include "Lasagna.hpp"
#include "IBackground.hpp"
#include <algorithm>

namespace RetroFuturaGUI
{
    class Window : public IBackground//, public IWidget
    {
    public:
        /// @brief Creates a native GLFW window with the given title and size.
        Window(std::string_view name, const i32 width, const i32 height);
        Window() = delete;
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        auto operator =(const Window&) = delete;
        auto operator =(Window&&) = delete;
        ~Window() = default;

        /// @brief Returns whether the user has requested the window be closed.
        bool WindowShouldClose();

        /// @brief Handles input, resizing/dragging and draws the window's background, bar and layout.
        void Draw();

        /// @brief Sets the window's width and height.
        void SetWindowSize(i32 width, i32 height);

        /// @brief Sets the window's width.
        void SetWidth(i32 width);

        /// @brief Sets the window's height.
        void SetHeight(i32 height);

        /// @brief Sets the window bar's title text and font family.
        void SetWindowTitle(std::string_view title, std::string_view fontFamily);
        //void SetWindowTitleFont(std::string_view fontPath); separate later when fallback fonts are implemented

        /// @brief Loads an image and applies it as the window's glass-effect background texture, propagating it to the window bar elements.
        void SetBackgroundImage(std::string_view imagePath) override;

        /// @brief Sets the Lasagna grid used to lay out the window's content.
        void SetLasagna(Lasagna* lasagna);

        /// @brief Returns the OpenGL texture ID of the background image, if one is set.
        i32 GetBackgroundImageId() const;

        /// @brief Returns the underlying GLFW window handle.
        GLFWwindow* GetGlfwWindow() const;

        /// @brief Returns the window's projection.
        Projection* GetProjection() const;

        /// @brief Returns the window's title bar.
        WindowBar& GetWindowBar();

        /// @brief Shows or hides the window's title bar.
        void ShowWindowBar(const bool show);

        /// @brief Returns the window's name.
        const std::string& GetName() const;

    private:
        enum class ResizeEdge : u32
        {
            None,
            Left,
            Right,
            Top,
            Bottom,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight
        };

        // Window properties
        GLFWwindow* _window {nullptr};

        i32
            _width { 1280 },
            _height { 720 },
            _minWindowSpan { 100 },
            _windowPosX { 0 },
            _windowPosY { 0 };
        f64 
            _prevResizeX { 0.0 },
            _prevResizeY { 0.0 },
            _cursorPosX { 0.0 },
            _cursorPosY { 0.0 },
            _absoluteCursorPosX { 0.0 },
            _absoluteCursorPosY { 0.0 },
            _boundaryThreshold { 10.0 };
        std::unique_ptr<Projection> _projection;
        bool _windowBarOverlapsBG { false };
        MaximizeState _maximizeState { MaximizeState::RESTORE };
        glm::ivec2
            _preMaximizeSize { 0 },
            _preMaximizePos { 0 };
        std::string 
            _name,
            _windowTitle { "RetroFuturaGUI Window" };

        // Window Actions
        ResizeEdge _resizeEdge { ResizeEdge::None };
        bool 
            _isResizing { false },
            _isDragging { false };
        glm::vec2 
            _dragStartPos { 0.0f },
            _windowDragStartPos { 0.0f };
        glm::ivec2 _lastSize = { _width, _height };
        bool _windowSizeChanged { false };

        // todo: create a cursor manager once the TextBox widget exists
        static inline GLFWcursor* _resizeCursorHorizontal { nullptr };
        static inline GLFWcursor* _resizeCursorVertical { nullptr };
        static inline GLFWcursor* _resizeCursorTLBR { nullptr };
        static inline GLFWcursor* _resizeCursorTRBL { nullptr };
        static inline GLFWcursor* _defaultCursor { nullptr };
        static inline GLFWcursor* _cursorIcon { nullptr };
        bool _cursorsInitialized { false };

        // widgets
        std::unique_ptr<WindowBar> _windowBar;
        Lasagna* _lasagna;

        void createWindow();
        static void cursorPositionCallback(GLFWwindow* window, f64 xpos, f64 ypos);
        void setCursorPosition();
        void setCursorIcon();
        static void mouseButtonClickedCallback(GLFWwindow* window, i32 button, i32 action, [[maybe_unused]] i32 mods);
        void setResizeState(i32 button, i32 action, [[maybe_unused]] i32 mods);
        void resize();
        void drag();
        static void windowFocusCallback(GLFWwindow* window, i32 focused);
        void moveWindow(const i32 posX, const i32 posY);
        static void setAbsoluteCursorPosition(Window* self);
        void toggleMaximize();
        void updateProjection();
        void setupWindowBar();
        void destroyWindowBar();
    };
}