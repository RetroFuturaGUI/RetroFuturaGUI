#pragma once
#include "IncludeHelper.hpp"
#include "ITab.hpp"
#include "Frame.hpp"
#include "Icon.hpp"
#include "Plane.hpp"
#include "SvgTexture.hpp"
#include "Signal.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    class Window;
    struct ScreenRect;

    /// @brief Docks windows as tabs. A strip of fields along the top, each showing a window's title and a close button. Only the selected window's content is visible below. Dragging
    ///        a field sideways reorders the fields, dragging it out of the strip pulls its window out again, and dragging a free window by its bar over the strip drops it in at the end
    class WindowTab final : public ITab
    {
    public:
        /// @brief Constructs an empty WindowTab
        WindowTab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        WindowTab() = delete;
        WindowTab(const WindowTab&) = delete;
        WindowTab(WindowTab&&) = delete;
        auto operator =(const WindowTab&) = delete;
        auto operator =(WindowTab&&) = delete;

        /// @brief Lets every window go: each becomes a free window again where the content area was.
        ~WindowTab() override;

        /// @brief Handles presses and drags on the strip, then draws the strip and the selected window's content.
        void Draw() override;

    //Windows
        /// @brief Docks a window into a new tab at the end of the strip, and selects it. A window docked to another window's
        ///        edge leaves that group first, and one in another WindowTab leaves that.
        /// @return false when the window is null or already here, is a popup's window, is the window this WindowTab lives in, or
        ///         holds windows in tabs of its own.
        bool AddWindow(Window* window);

        /// @brief Docks a window into a new tab at the given place among the fields, and selects it.
        /// @return false in the same cases as AddWindow.
        bool InsertWindow(Window* window, const uSize index);

        /// @brief Takes a window out of its tab. It becomes a free window again where the content area was, at the size it had before.
        /// @return false when the window isn't in this WindowTab.
        bool RemoveWindow(Window* window);

        /// @brief Shows the given window's tab.
        void SelectWindow(Window* window, const bool emitSignal);

        /// @brief Returns the window whose tab shows, or nullptr when there are no tabs.
        Window* GetSelectedWindow() const;

        /// @brief Returns the window of the tab at the given place, or nullptr when out of range.
        Window* GetWindow(const uSize index) const;

        /// @brief Returns how many windows are docked here.
        uSize GetWindowCount() const;

        /// @brief Returns the window this WindowTab lives in, which draws the selected window.
        Window* GetHostWindow() const;

    //Signals
        /// @brief Connects a slot called when another tab is selected, with its window, or with nullptr once the last tab is gone.
        void Connect_OnSelectionChanged(const typename Signal<Window*>::Slot& slot, const bool async);
        void Disconnect_OnSelectionChanged(const typename Signal<Window*>::Slot& slot);

        /// @brief Connects a slot called with each window docked into a tab.
        void Connect_OnWindowAdded(const typename Signal<Window*>::Slot& slot, const bool async);
        void Disconnect_OnWindowAdded(const typename Signal<Window*>::Slot& slot);

        /// @brief Connects a slot called with each window taken out of its tab, whether by dragging it out, by RemoveWindow or
        ///        because it closed.
        void Connect_OnWindowRemoved(const typename Signal<Window*>::Slot& slot, const bool async);
        void Disconnect_OnWindowRemoved(const typename Signal<Window*>::Slot& slot);

    private:
        friend class Window; //resets _isShown each frame, finds the strip under a dragged window and shows a preview field for it

    //Interaction
        /// @brief Marks the WindowTab shown, follows the pointer on the strip, then passes the input on to the selected window.
        void interact() override;

        /// @brief Has the window in the selected tab take the frame's input in the content area.
        void interactSelectedWindow();

        /// @brief Whether the cursor left the strip far enough to pull the dragged field out: 1.5 title font sizes above or
        ///        below it, a little more after a long sideways drag, or 2.2 past the first or the last field (ImGui's rule).
        bool isPullingOut(const uSize index, const glm::vec2& point) const;

        /// @brief Takes the field's window out, at its old size, with the cursor holding it where it held the field, and lets it
        ///        follow the cursor until the button is let go.
        void pullOut(const uSize index, const glm::vec2& point);

        /// @brief The window of the field, or nullptr.
        Window* windowOf(const uSize index) const;

    //Layout
        ScreenRect getContentArea() const;

        /// @brief Moves the drop preview's field toward its slot over the Position transition. It grows from nothing.
        void syncDropPreview();

        /// @brief Converts a point in this window's coordinates to the screen's, with y running downward.
        glm::ivec2 toScreen(const glm::vec2& point) const;

    //Drawing
        void drawSelectedWindow();
        void drawDropPreview();

        /// @brief Lays a field's title out again when its window's title changed.
        void syncTitles();

    //ITab
        /// @brief Keeps the press that selected it from clicking what lies under it, and has the layout file keep the selection
        void fieldSelected(IHierarchyNode* item, const bool emitSignal) override;
        void selectionCleared() override;

        /// @brief Asks the field's window to close, as its own close button does. Whoever owns it destroys it, which takes it out
        void closeField(const uSize index) override;

        /// @brief Pulls the field's window out once the cursor left the strip far enough.
        bool dragsOutOfStrip(const uSize index, const glm::vec2& point) override;

        /// @brief Has the layout file keep the tabs' order.
        void fieldsReordered() override;

        /// @brief While a window is dragged over the strip, its preview field counts too, so the fields narrow to make room for it
        uSize getSlotCount() const override;

        void titleFontChanged() override;

    //Called by Window
        /// @brief Whether the strip lies under the given point on the screen, and takes windows: drawn this frame and enabled.
        bool isStripUnder(const glm::ivec2& screenPoint) const;

        /// @brief The content area on the screen, with y running downward: left, top, width, height.
        glm::ivec4 getContentScreenRect() const;

        /// @brief Draws a field with the given title at the end of the strip, where a dragged window would drop.
        void showDropPreview(const std::string& title);
        void hideDropPreview();

    //Elements
        Box _previewFieldStyle { &_projection }; //the selected look at half its opacity, for the field a dropped window would take
        std::unique_ptr<Text> _previewTitle { nullptr };
        TransitionValue<glm::vec2> _previewShown {}; //the drop preview field's left edge and width as drawn
        bool _isPreviewShown { false }; //a dropped window's field starts where the preview was

    //Logic
        static constexpr f32
            _pullOutDistance { 1.5f }, //how far above or below the strip the cursor pulls a field out, in title font sizes
            _pullOutPastEndDistance { 2.2f }; //how far past the first or last field it pulls one out, in title font sizes
        bool
            _showsDropPreview { false },
            _isShown { false }; //interacted since its window's frame began, so it shows and a dragged window may drop into it

    //Signals
        Signal<Window*>
            _onSelectionChanged,
            _onSelectionChangedAsync,
            _onWindowAdded,
            _onWindowAddedAsync,
            _onWindowRemoved,
            _onWindowRemovedAsync;
    };
}
