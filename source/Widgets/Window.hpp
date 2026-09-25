#pragma once
#include "IncludeHelper.hpp"
#include "Label.hpp"
#include "WindowBar.hpp"
#include "Image.hpp"
#include "Lasagna.hpp"
#include "IBackground.hpp"
#include "IHierarchyNode.hpp"
#include <algorithm>

namespace RetroFuturaGUI
{
    class Scene;
    enum class DockEdge : u32;

    /// @brief An axis-aligned region of the window in projection space, with its origin at the bottom-left corner. Widgets take their position as a centre, so Center() converts.
    struct ScreenRect
    {
        glm::vec2
            _Origin { 0.0f },
            _Extent { 0.0f };

        glm::vec2 Center() const { return _Origin + _Extent * 0.5f; }
    };

    class Window final : public IBackground, public IHierarchyNode
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

        /// @brief Sets the Lasagna used to lay out the window's content. It is fitted into the client area left over once the window bar and every docked scene have reserved theirs
        void SetLasagna(Lasagna* lasagna);

        /// @brief Adds a scene to the window's draw order, after the ones already added. Docked scenes reserve space in this order, so the first one to claim an edge also owns the corner where two claims meet.
        void AddScene(Scene* scene);

        /// @brief Removes a scene from the window's draw order, leaving the order of the rest intact.
        void RemoveScene(Scene* scene);

        /// @brief Recomputes the client area and re-lays out the root Lasagna and every scene. Called automatically on resize and whenever a scene changes what it reserves.
        void UpdateLayout();

        /// @brief When true the window bar is drawn over the client area rather than reserving space across its edge - for designs where content is meant to run underneath it.
        void SetWindowBarOverlaps(const bool overlaps);
        
        bool WindowBarOverlaps() const;

        /// @brief The client area left after the window bar and every docked scene took their share.
        const ScreenRect& GetClientRect() const;

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
        const std::string& GetName() const override;

        /// @brief Returns nullptr: a window is always the root of its path.
        const IHierarchyNode* GetParent() const override;

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
            _isDragging { false },
            _wasMouseButtonDown { false };
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
        Lasagna* _lasagna { nullptr };

        // scenes - borrowed, owned by whoever loaded them
        std::vector<Scene*> _scenes {};
        ScreenRect _clientRect {};

        /// @brief Shrinks the client area across the window bar's edge, unless the bar overlaps.
        void reserveWindowBar(ScreenRect& client) const;

        /// @brief Takes a strip of `thickness` off the given edge of `client` and returns it,
        ///        shrinking `client` by what it took. Never takes more than is left.
        static ScreenRect reserveEdge(const DockEdge edge, const f32 thickness, ScreenRect& client);

        void createWindow();
        void pollInput();
        bool getCursorPosition(i32& x, i32& y) const;
        void setCursorPosition();
        void setCursorIcon();
        void setResizeState(i32 button, i32 action, [[maybe_unused]] i32 mods);
        void resize();
        void drag();
        void moveWindow(const i32 posX, const i32 posY);
        static void setAbsoluteCursorPosition(Window* self);
        void toggleMaximize();
        void updateProjection();
        void setupWindowBar();
        void destroyWindowBar();
    };
}