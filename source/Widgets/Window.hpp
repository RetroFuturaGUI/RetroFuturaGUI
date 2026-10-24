#pragma once
#include "IncludeHelper.hpp"
#include "Label.hpp"
#include "Image.hpp"
#include "Lasagna.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IHierarchyNode.hpp"
#include "IPointerFilter.hpp"
#include "IPopup.hpp"
#include "DockEdge.hpp"
#include "Signal.hpp"
#include "CursorIcon.hpp"
#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace RetroFuturaGUI
{
    class Scene;
    class WindowTab;

    /// @brief An axis-aligned region of the window in projection space, with its origin at the bottom-left corner. Widgets take their position as a centre, so Center() converts.
    struct ScreenRect
    {
        glm::vec2
            _Origin { 0.0f },
            _Extent { 0.0f };

        glm::vec2 Center() const { return _Origin + _Extent * 0.5f; }
    };

    class Window final : public IBackground, public IBorder, public IHierarchyNode, public IPointerFilter
    {
    public:
        /// @brief Creates a native GLFW window with the given title and size.
        Window(std::string_view name, const i32 width, const i32 height);
        Window() = delete;
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        auto operator =(const Window&) = delete;
        auto operator =(Window&&) = delete;
        /// @brief Destroys the native window. The widgets of its scenes have to be gone by then, and destroyed with this window's context current.
        ~Window();

        /// @brief Returns whether the user has requested the window be closed. A window under a popup drops the request and
        ///        stays open, like a native window under a modal dialog.
        bool WindowShouldClose();

        /// @brief Makes the window's GL context current, then handles input and resizing, and draws the window's background, its
        ///        root Lasagna and its scenes. Then draws its popups, and closes the top one if it was asked to.
        void Draw();

        /// @brief Sets the window's width and height.
        void SetWindowSize(i32 width, i32 height);

        /// @brief Sets the window's width.
        void SetWidth(i32 width);

        /// @brief Sets the window's height.
        void SetHeight(i32 height);

        /// @brief Sets the window's native title, the one the taskbar and the window switcher show. A title drawn inside the window belongs to whichever scene draws it.
        void SetWindowTitle(std::string_view title);

        /// @brief Loads an image and applies it as the window's glass-effect background texture.
        void SetBackgroundImage(std::string_view imagePath) override;

        /// @brief Sets the Lasagna used to lay out the window's content. It is fitted into the client area left over once every docked scene has reserved its strip.
        void SetLasagna(Lasagna* lasagna);

        /// @brief Adds a scene to the window's draw order, after the ones already added. Docked scenes reserve space in this order, so the first one to claim an edge also owns the corner where two claims meet.
        void AddScene(Scene* scene);

        /// @brief Removes a scene from the window's draw order, leaving the order of the rest intact.
        void RemoveScene(Scene* scene);

        /// @brief Recomputes the client area and re-lays out the root Lasagna and every scene. Called automatically on resize and whenever a scene changes what it reserves.
        void UpdateLayout();

        /// @brief The client area left after every docked scene took its share.
        const ScreenRect& GetClientRect() const;

        /// @brief Returns the OpenGL texture ID of the background image, if one is set.
        i32 GetBackgroundImageId() const;

        /// @brief Returns the underlying GLFW window handle.
        GLFWwindow* GetGlfwWindow() const;

        /// @brief Returns the window's projection.
        Projection* GetProjection() const;

        /// @brief Returns the window's name.
        const std::string& GetName() const override;

        /// @brief Returns nullptr: a window is always the root of its path.
        const IHierarchyNode* GetParent() const override;

        /// @brief Maximizes the window, or restores it to its previous size and position if it is maximized already.
        void ToggleMaximize();

        /// @brief Returns whether the window is maximized. GLFW is asked each time rather than a flag being kept, so the answer cannot drift from the real window.
        bool IsMaximized() const;

        /// @brief Sets the radii of the background's and the border's corners, clockwise from the top-left. Docking may square
        ///        some of them, see SetNoCornerRadiiWhenDocked.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Returns the radii the background and border are drawn with: the ones set, with the corners where docked windows
        ///        meet squared. A widget lining the window's edge, such as a window bar, can match its own corners to them.
        glm::vec4 GetDrawnCornerRadii() const;

    //Docking
        /// @brief Docks a window to one of this window's edges, after the ones already docked there. The windows on one edge
        ///        share its whole length, minus the docking padding between them, and keep their own depth. From then on the
        ///        group moves together: moving or resizing any of its windows carries the others along.
        /// @return false when the window is null, this window, already has a host (the window it is docked to), or is this
        ///        window's host or a host further up.
        bool DockWindow(Window* window, const DockEdge edge);

        /// @brief Takes this window out of its docking group. It leaves the window it is docked to, whose other docked windows
        ///        grow to fill the gap, and lets go of the windows docked to it where they are. Every window that stops being
        ///        docked gets back the length it had before. A window docked into a WindowTab leaves the tab.
        void Undock();

        /// @brief Returns whether this window is docked to another window or has windows docked to it.
        bool IsDocked() const;

        /// @brief Sets the gap this window keeps to the windows it is docked with. The windows on one edge, and the window whose
        ///        edge it is, all keep the largest padding among them, so it doesn't matter which window was docked onto which.
        void SetDockingPadding(const f32 padding);

        /// @brief Sets, for every window, whether the corners where docked windows meet are squared.
        static void SetNoCornerRadiiWhenDocked(const bool noCornerRadii);

        /// @brief Returns the window that drives the given GLFW window, or nullptr when there is none.
        static Window* FromGlfwWindow(GLFWwindow* glfwWindow);

        /// @brief Tells the window that the user started dragging it, from a window bar. While it is docked to nothing, dragging
        ///        it near another window's edge shows a translucent preview of where it would dock, and letting go docks it there.
        void BeginDrag();

        /// @brief Tells the window that the drag ended. It docks where the preview showed, if a preview was showing.
        void EndDrag();

    //Tabs
        /// @brief Returns whether this window is docked into a WindowTab.
        bool IsInTab() const;

        /// @brief Returns the window's native title
        const std::string& GetWindowTitle() const;

        /// @brief Returns the native window keyboard input arrives at: the host's while this window is docked into a WindowTab,
        ///        since its own native window is hidden then and never has the focus, otherwise its own.
        GLFWwindow* GetKeyboardWindow() const;

        /// @brief Makes key presses arrive at this window
        void TakeKeyboardInput() const;

        /// @brief The native window keyboard input for the given GLFW window's widgets arrives at, see GetKeyboardWindow. Widgets
        ///        only hold the GLFW window, so they ask through this. A GLFW window without a Window answers for itself.
        static GLFWwindow* KeyboardWindowOf(GLFWwindow* glfwWindow);

        /// @brief Asks for the icon the mouse cursor shows this frame, over the current window
        static void RequestCursorIcon(GLFWwindow* glfwWindow, const PlatformBridge::CursorIcon icon);

        /// @brief Where the given GLFW window's (0, 0) lies in the framebuffer being drawn: the corner of the content area while a
        ///        WindowTab's window draws it, otherwise (0, 0). Scissor boxes count framebuffer pixels, so a widget setting one
        ///        adds this.
        static glm::ivec2 DrawOriginOf(GLFWwindow* glfwWindow);

    //Popups
        /// @brief Opens a popup over this window and takes ownership of it. A window has one popup stack: a popup opened from a
        ///        popup's window goes on top of the same stack. Only the top popup is interactiable
        /// @return false when the popup is null or already open, or when this is the window of a popup that isn't open yet.
        bool PushPopup(std::unique_ptr<IPopup> popup);

        /// @brief Returns whether popups are open over this window.
        bool HasPopups() const;

        /// @brief Returns whether a popup lies over this window
        bool IsBlockedByPopup() const;

        /// @brief Takes all interactions away from this window's widgets while a popup lies over it
        void FilterPointer(PointerState& pointer) const override;

    //Dropped files
        /// @brief Connects a slot to files dropped on the window, from another program or another of the app's windows: their
        ///        UTF-8 paths, and where the pointer was, in window coordinates. Sent from the next draw, never from inside the
        ///        system's message handling. A window under a popup takes no drops, like a window under a native modal dialog.
        void Connect_OnFilesDropped(const typename Signal<std::vector<std::string>, glm::vec2>::Slot& slot, const bool async);

        void Disconnect_OnFilesDropped(const typename Signal<std::vector<std::string>, glm::vec2>::Slot& slot);

    private:
        /// @brief A screen rectangle in screen coordinates, y running downward: where a window is, or would be once docked.
        struct DockRect
        {
            glm::ivec2
                _Position { 0 },
                _Size { 0 };
        };

        /// @brief Where a dragged window docks when it is let go, if anywhere.
        struct DockSnap
        {
            Window* _Target { nullptr };
            DockEdge _Edge { DockEdge::Top };
            uSize _Index { 0 }; //its place among the windows already docked to that edge
            WindowTab* _Tab { nullptr }; //set when it drops into one of the target's WindowTabs instead, at the end of its strip
        };

        /// @brief What a window is for. Docking previews and popups' windows start hidden and stay out of docking and the window list.
        enum class WindowRole : u8
        {
            Normal,
            DockPreview, //a bare translucent window that stays out of input, and only shows while a dragged window is near an edge it can dock to
            Popup //an IPopup's window
        };

        /// @brief Creates a window in the given role.
        Window(std::string_view name, const i32 width, const i32 height, const WindowRole role);

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

        // The first window created. Every later one shares its textures, buffers and shaders, which are only loaded once
        static inline GLFWwindow* _shareWindow { nullptr };

    // Docking
        std::list<Window*>
            _bottomDockedWindows {},
            _rightDockedWindows {},
            _topDockedWindows {},
            _leftDockedWindows {};
        f32 _dockingPadding { 0.0f };
        static inline bool _noCornerRadiiWhenDocked { true };
        Window* _dockHost { nullptr };
        DockEdge _dockHostEdge { DockEdge::Top };
        i32 _undockedLength { 0 }; //length before docking for restore when undocking
        glm::ivec2 //Where the docking layout last saw or put the window, so a move or resize from anywhere else stands out
            _dockedPosition { 0 },
            _dockedSize { 0 };
        bool
            _dockingLayoutPending { false },//The layout had to skip this window while it was minimized or maximized, so it lays out its group once it is restored
            _isDragged { false };
        WindowRole _role { WindowRole::Normal };
        glm::ivec2 _dragStartPosition { 0 };
        DockSnap _dockSnap {};
        static constexpr i32 _snapDistance { 32 };
        std::unique_ptr<Window> _dockPreview { nullptr }; //created the first time this window is dragged near an edge
        glm::vec4
            _cornerRadii { 0.0f },
            _appliedCornerRadii { -1.0f }; //what the background and border last got, so they are only set again on a change

    // Popups
        std::vector<std::unique_ptr<IPopup>> _popups {};
        Window* _popupHost { nullptr }; //set on popups' windows only: the window holding the popup stack
        bool
            _ignoreHeldPress { false }, //the window just took the mouse, and the button may still be down from the click that caused it
            _showAfterDraw { false }, //a popup's window that just opened, or a window that just left a tab, and shows once it has drawn
            _hasPointer { false }; //the mouse reaches this window: it lies on top under the cursor, or took the press that is still held

    // Dropped files
        /// @brief Files dropped on the window, kept until the next draw sends them.
        struct DroppedFiles
        {
            std::vector<std::string> _Paths {};
            glm::vec2 _Position { 0.0f }; //window coordinates
        };

        std::vector<DroppedFiles> _droppedFiles {};
        Signal<std::vector<std::string>, glm::vec2>
            _onFilesDropped,
            _onFilesDroppedAsync;

        /// @brief GLFW's drop callback: keeps the drop for the next draw.
        static void dropCallback(GLFWwindow* glfwWindow, const int pathCount, const char* paths[]);

        /// @brief Sends the files dropped since the last draw, unless a popup lies over the window.
        void emitDroppedFiles();

    // Tabs
        WindowTab* _tabHost { nullptr }; //the WindowTab this window is docked into/which window draws it
        std::vector<WindowTab*> _windowTabs {}; //the WindowTabs in this window's scenes, registered by them
        glm::ivec2
            _untabbedSize { 0 },
            _drawOrigin { 0 },
            _cursorGrabPoint { 0 }; //where the cursor holds the window while it follows the cursor after being pulled out of a tab
        glm::vec4 _tabCornerRadii { 0.0f }; //the content area's radii, which the window takes while it is in a tab
        bool _followsCursor { false }; //whether window should move when pulled out off a tab

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
        std::string
            _name,
            _windowTitle { "RetroFuturaGUI Window" };

        // Window Actions
        ResizeEdge _resizeEdge { ResizeEdge::None };
        bool
            _isResizing { false },
            _wasMouseButtonDown { false };
        glm::ivec2 _lastSize = { _width, _height };
        bool _windowSizeChanged { false };
        PlatformBridge::CursorIcon
            _cursorIcon { PlatformBridge::CursorIcon::Arrow }, //the last icon handed to PlatformBridge, which shows the arrow until the first
            _requestedCursorIcon { PlatformBridge::CursorIcon::Arrow }; //what a widget under the mouse asked for this frame

        // widgets
        Lasagna* _lasagna { nullptr };

        // scenes - borrowed, owned by whoever loaded them
        std::vector<Scene*> _scenes {};
        ScreenRect _clientRect {};

        /// @brief Takes a strip of `thickness` off the given edge of `client` and returns it,
        ///        shrinking `client` by what it took. Never takes more than is left.
        static ScreenRect reserveEdge(const DockEdge edge, const f32 thickness, ScreenRect& client);

        void createWindow();

        /// @brief Reads the mouse for this frame: whether it reaches this window and where the cursor is. Then sets the cursor, and starts or ends a resize on a press or release.
        void pollInput();

        /// @brief Whether this window is the topmost one under the cursor, of any application's.
        bool isUnderMouse() const;
        bool getCursorPosition(i32& x, i32& y) const;
        void setCursorPosition();
        void setCursorIcon();
        void setResizeState(i32 button, i32 action, [[maybe_unused]] i32 mods);
        void resize();

        /// @brief Takes over the native window's size when it changed: the projection, the layout and the background's and border's
        ///        geometry, then the viewport.
        void updateProjection();

        /// @brief Everything updateProjection does but the viewport, which belongs to whichever window's frame is being drawn.
        /// @return whether the size changed.
        bool updateSize();

        /// @brief Draws the corner radii, background, root Lasagna, scenes, frame and tooltip into whatever framebuffer and viewport are set.
        void drawContent();

        /// @brief The edge or corner a press at the given window coordinates would resize, or None.
        ResizeEdge resizeEdgeAt(const f64 x, const f64 y) const;

        /// @brief Whether that edge may be resized. A docked window's host decides its length and where its near edge sits, so
        ///        only its far edge is left.
        bool canResize(const ResizeEdge edge) const;

        /// @brief Draws the border over the scenes, when it has a width.
        void drawFrame();

    //Docking
        /// @brief The list of windows docked to the given edge.
        std::list<Window*>& dockedWindows(const DockEdge edge);
        const std::list<Window*>& dockedWindows(const DockEdge edge) const;

        /// @brief The window at the root of this window's docking group, which is this one when it is docked to nothing.
        Window* dockRoot();

        /// @brief Whether the given window is this one's host, or the host of a window further up the chain.
        bool isDockedTo(const Window* window) const;

        /// @brief Leaves the window this one is docked to, which then lays out the windows left on that edge.
        void leaveDockHost();

        /// @brief Lets go of every window docked to this one, where they are.
        void releaseDockedWindows();

        /// @brief Gives the window back the length it had along its host's edge before docking.
        void restoreUndockedLength();

        /// @brief Places and sizes the windows docked to this one, then the ones docked to those.
        void layoutDockedWindows();

        /// @brief Moves and resizes the window for the docking layout, and remembers where it put it.
        void setDockedGeometry(const glm::ivec2& position, const glm::ivec2& size);

        /// @brief Called after each draw: when the window was moved or resized since the docking layout last placed it, the
        ///        rest of its group follows.
        void followDocking();

        /// @brief Whether the window is minimized or maximized, which keeps it out of the docking layout until it is restored.
        bool isMinimizedOrMaximized() const;

        /// @brief The corners where a docked window meets this one, clockwise from the top-left like the radii.
        glm::bvec4 squaredCorners() const;

        /// @brief Hands the corner radii to the background and border, with the corners where docked windows meet squared.
        void applyCornerRadii();

        /// @brief Marks the two corners at the ends of an edge.
        static void squareEdgeCorners(glm::bvec4& squared, const DockEdge edge);

        /// @brief The edge across from the given one.
        static DockEdge oppositeEdge(const DockEdge edge);

        /// @brief Docks a window at the given place among the ones on the edge. DockWindow appends.
        bool dockWindowAt(Window* window, const DockEdge edge, const uSize index);

        /// @brief Where the window is on screen.
        DockRect screenRect() const;

        /// @brief The gap on one of this window's edges, in whole pixels: the largest docking padding among this window, the windows
        ///        on that edge, and the one about to join them, if any. So the gap doesn't depend on which window hosts the other.
        i32 edgeGap(const DockEdge edge, const Window* joining) const;

        /// @brief The window's own size away from the given edge of its host: its width beside it, its height above or below.
        i32 dockDepth(const DockEdge edge) const;

        /// @brief Where a window of the given depth goes when it sits at index among count windows on one of this window's edges.
        ///        The docking layout and the docking preview both place by it, so the preview shows exactly where it lands.
        DockRect dockSlot(const DockRect& host, const DockEdge edge, const uSize index, const uSize count, const i32 depth, const i32 gap) const;

        /// @brief The place a window would take among the ones already on the edge: before every one whose middle lies past its own.
        uSize dockIndexFor(const DockRect& dragged, const DockEdge edge) const;

        /// @brief Called after each draw: while the window is dragged, finds where it would dock and shows the preview there.
        void updateDockSnap();

        /// @brief The closest edge of another window the dragged one is near enough to dock to, if any. A WindowTab strip under the
        ///        cursor wins over every edge.
        DockSnap findDockSnap() const;

        /// @brief The WindowTab whose strip lies under the cursor, if the dragged window may drop into it.
        DockSnap findTabSnap() const;

        /// @brief Shows the docking preview over the given place on the screen, rounded with the given radii, creating it the first time.
        void showDockPreview(const DockRect& slot, const glm::vec4& cornerRadii);

        /// @brief This window's corner radii with the corners that would meet a window it docks to on the given edge squared, the
        ///        way they will be once it is docked.
        glm::vec4 cornerRadiiDockedAt(const DockEdge edge) const;

        /// @brief Hides the docking preview, if it shows.
        void hideDockPreview();

        /// @brief How far the dragged window is from where it would sit docked to the host's edge, across that edge.
        static i32 dockDistance(const DockRect& dragged, const DockRect& host, const i32 gap, const DockEdge edge);

        /// @brief How far the dragged window and the host overlap along the edge. Without overlap there is nothing to dock to.
        static i32 dockOverlap(const DockRect& dragged, const DockRect& host, const DockEdge edge);

        /// @brief Every window except docking previews and popups' windows, so a dragged window can find the ones it could dock
        ///        to. Never destroyed, so a window destroyed late in shutdown can still take itself off it.
        static std::vector<Window*>& allWindows();

    //Popups
        /// @brief Draws every popup on the stack, bottom first, then closes the top one if it was asked to.
        void drawPopups();

        /// @brief Closes the top popup if its window was asked to close: takes it off the stack, sends its result, then destroys it.
        void closeTopPopup();

        /// @brief The window of the top popup, or this window when no popup is open.
        Window* topPopupWindow();

        /// @brief Drops a request to close while a popup lies over this window.
        void refuseCloseWhileBlocked();

        /// @brief Centers this window over the given one.
        void centerOver(const Window* window);

        /// @brief Makes the given window this one's native owner, which keeps this one above it.
        void setNativeOwner(const Window* owner);

    //Tabs
        /// @brief Docks this window into the WindowTab: it leaves its docking group, its native window hides, and from then on the
        ///        WindowTab's window draws it.
        /// @return false for popups' windows and docking previews, for the WindowTab's own window, for a window holding windows in
        ///         its own WindowTabs, and when the WindowTab's window sits in a tab itself.
        bool enterTab(WindowTab* tab);

        /// @brief Takes this window out of its WindowTab. It gets back the size it had before, with its top-left corner at the given
        ///        place on the screen, and shows once it has drawn a frame of its own.
        void leaveTab(const glm::ivec2& position);

        /// @brief Draws this window into its WindowTab's content area, with the WindowTab's window's context current: viewport and
        ///        scissor box set to the content area, and the hidden native window kept right under it, so the cursor reads the
        ///        same as on the window itself.
        /// @param contentArea the content area in the drawing window's framebuffer pixels, y running upward
        /// @param screenPosition the content area's top-left corner on the screen
        void drawInHost(const ScreenRect& contentArea, const glm::ivec2& screenPosition, const glm::vec4& cornerRadii);

        /// @brief Moves the window with the cursor, held at the given point, until the left button is let go, then ends the drag,
        ///        which docks or tabs it where its preview showed. For a window just pulled out of a tab, since the press that
        ///        drags it began in another window.
        void beginCursorDrag(const glm::ivec2& grabPoint);

        /// @brief Called after each draw: keeps the cursor at the grab point while the window follows it.
        void followCursor();

        /// @brief Whether the scene draws and takes its space: it is active, and not hidden while this window is in a tab.
        bool isSceneShown(const Scene* scene) const;

        /// @brief Whether any of this window's WindowTabs holds windows.
        bool holdsTabbedWindows() const;

        /// @brief The window whose WindowTab this one is docked into, or nullptr.
        Window* tabHostWindow() const;

        /// @brief Whether the WindowTab still lives in one of the windows, so a pointer kept from an earlier frame is safe to use.
        static bool isRegisteredWindowTab(const WindowTab* tab);

        friend class IPopup; //creates its window through the private constructor
        friend class WindowTab; //registers itself, docks windows into itself and draws the selected one
    };
}