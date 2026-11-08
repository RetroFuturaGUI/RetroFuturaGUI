#pragma once
#include "Background.hpp"
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "Border.hpp"
#include "Box.hpp"
#include "CloseButton.hpp"
#include "Frame.hpp"
#include "Plane.hpp"
#include "SvgTexture.hpp"
#include "Icon.hpp"
#include "ResourceManager.hpp"
#include "Text.hpp"
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
    class WindowTab final : public IWidget
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
        ~WindowTab();

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

    //Strip
        /// @brief Sets the height of the strip the fields sit in.
        void SetStripHeight(const f32 height);

        /// @brief Sets how narrow and how wide a field may get. Fields share the strip evenly within that range, and the ones
        ///        that no longer fit at the minimum width are left out.
        void SetFieldWidthRange(const f32 minimumWidth, const f32 maximumWidth);

        /// @brief Sets the gap between fields, and above them.
        void SetFieldSpacing(const f32 spacing);

    //Titles
        /// @brief Sets the font of the titles. Without one they don't show.
        void SetTitleFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

        /// @brief Returns the title font size in the given unit.
        f32 GetTitleFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets a title's colors for Enabled, Disabled, Hover or Clicked. The selected window's title shows the same colors as
        ///        the others, so Selected is ignored
        void SetTitleColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Sets the space between a field's left edge and its title.
        void SetTitlePadding(const f32 padding);

    //Close buttons
        /// @brief Sets the width and height of the close buttons.
        void SetCloseButtonSize(const f32 size);

        /// @brief Returns the close button every field draws: its background, transparent until hovered by default, and its
        ///        cross. Each field's button shows that field's state. Its corners round with a quarter of SetCloseButtonSize
        CloseButton& GetCloseButton();

        /// @brief Returns the close button every field draws (see the other overload)
        const CloseButton& GetCloseButton() const;

        /// @brief Returns the look of the fields: Enabled, Hover, Clicked and Disabled, and Selected for the selected window's field,
        ///        which usually matches the content so the two join. Rounded at the top, without a border by default
        Box& GetFieldStyle();

        /// @brief Returns the look of the fields; see the other overload.
        const Box& GetFieldStyle() const;

    //Content
        /// @brief Sets the corner radii of the content area, clockwise from the top-left. The docked window rounds its own
        ///        corners to them, to match the rounded corners of the window this WindowTab fills, for instance
        void SetContentCornerRadii(const glm::vec4& radii);

        /// @brief Returns the corner radii of the content area.
        const glm::vec4& GetContentCornerRadii() const;
        
        /// @brief Returns the WindowTab's background
        Background& GetBackground();
        
        /// @brief Returns the WindowTab's background
        const Background& GetBackground() const;
        
        /// @brief Returns the WindowTab's border
        Border& GetBorder();
        
        /// @brief Returns the WindowTab's border
        const Border& GetBorder() const;

    protected:
        /// @brief Sets the center of the whole WindowTab, strip and content area together.
        void applyPosition(const glm::vec3& position) override;

        /// @brief Sets the size of the whole WindowTab, strip and content area together.
        void applySize(const glm::vec3& size) override;

    private:
        friend class Window; //resets _isShown each frame, finds the strip under a dragged window and shows a preview field for it

        /// @brief One tab: the window it docks and the title its field shows.
        struct Field
        {
            Window* _Window { nullptr };
            std::unique_ptr<Text> _Title { nullptr };
            std::string _ShownTitle {}; //what _Title shows, so it is only laid out again when the window's title changes
            TransitionValue<glm::vec2> _Shown {}; //the field's left edge and width as drawn, on their way to its slot
            bool _HasShown { false }; //a field not shown yet stands in its slot at once
        };

        /// @brief The colors for every state a field and its parts can be in.
        struct FieldColors
        {
            std::vector<glm::vec4>
                _Enabled { glm::vec4(0.0f) },
                _Hover { glm::vec4(0.0f) },
                _Clicked { glm::vec4(0.0f) },
                _Disabled { glm::vec4(0.0f) },
                _Selected { glm::vec4(0.0f) };
        };

    //Interaction
        /// @brief Marks the WindowTab shown, follows the pointer on the strip, then passes the input on to the selected window.
        void interact() override;

        /// @brief Selects or closes on a press, and drags the pressed field until the button is let go.
        void followPointer();

        /// @brief Has the window in the selected tab take the frame's input in the content area.
        void interactSelectedWindow();

        /// @brief Selects the field and remembers where it was grabbed, in case the press turns into a drag.
        void pressField(const uSize index, const glm::vec2& point);

        /// @brief Moves the pressed field with the cursor, reordering the fields, or pulls it out once it leaves the strip.
        void dragField(const glm::vec2& point);

        /// @brief Whether the cursor left the strip far enough to pull the dragged field out: 1.5 title font sizes above or
        ///        below it, a little more after a long sideways drag, or 2.2 past the first or the last field (ImGui's rule).
        bool isPullingOut(const uSize index, const glm::vec2& point) const;

        /// @brief Takes the field's window out, at its old size, with the cursor holding it where it held the field, and lets it
        ///        follow the cursor until the button is let go.
        void pullOut(const uSize index, const glm::vec2& point);

        /// @brief Asks the field's window to close, as its own close button does. Whoever owns it destroys it, which takes it out.
        void closeWindow(const uSize index) const;

        /// @brief The field under the point, or _kNoField.
        uSize fieldAt(const glm::vec2& point) const;

        /// @brief The state a field draws in, given what the pointer is doing.
        ColorState fieldState(const uSize index) const;

    //Layout, in window coordinates with y running upward. A rectangle is (left, bottom, width, height).
        glm::vec4 getStripRect() const;
        ScreenRect getContentArea() const;
        f32 getFieldWidth() const;

        /// @brief Where the field at the given place sits, ignoring a drag.
        glm::vec4 slotRect(const uSize slot) const;

        /// @brief Where the field is drawn: on its way to its slot, or under the cursor while it is dragged.
        glm::vec4 fieldRect(const uSize index) const;

        /// @brief Returns the left edge and width a field is moving toward: its slot's, or where the cursor drags it.
        glm::vec2 getFieldTarget(const uSize index) const;

        /// @brief Moves every field and the drop preview's toward their slots over the Position transition. The dragged field
        ///        follows the cursor at once, and the preview field grows from nothing.
        void syncFieldRects();

        /// @brief The place along the strip whose slot the given x lies over.
        uSize slotAt(const f32 x) const;

        /// @brief Whether the field fits in the strip. The ones that don't are left out.
        bool fitsInStrip(const glm::vec4& field) const;

        glm::vec4 closeButtonRect(const glm::vec4& field) const;
        glm::vec4 titleRect(const glm::vec4& field) const;

        /// @brief Converts a point in this window's coordinates to the screen's, with y running downward.
        glm::ivec2 toScreen(const glm::vec2& point) const;

    //Drawing
        void drawFields();
        void drawField(const uSize index, const f32 z);
        void drawFieldShape(const glm::vec4& rect, const f32 z, Box& style, const ColorState state);

        /// @brief The state a field's look shows: Disabled, Selected for the selected window's field, or the field's own
        static ColorState fieldStyleState(const ColorState state, const bool selected);

        /// @brief Gives the field look its default colors, corners and border width
        void setFieldStyleDefaults();
        void drawCloseButton(const glm::vec4& field, const f32 z, const ColorState state);
        void drawTitle(Text* title, const glm::vec4& field, const f32 z, const std::vector<glm::vec4>& colors);
        void drawSelectedWindow();
        void drawDropPreview();

        /// @brief Lays a field's title out again when its window's title changed.
        void syncTitles();

        void applyTitleFont(Text* title) const;

    //Called by Window
        /// @brief Whether the strip lies under the given point on the screen, and takes windows: drawn this frame and enabled.
        bool isStripUnder(const glm::ivec2& screenPoint) const;

        /// @brief The content area on the screen, with y running downward: left, top, width, height.
        glm::ivec4 getContentScreenRect() const;

        /// @brief Draws a field with the given title at the end of the strip, where a dragged window would drop.
        void showDropPreview(const std::string& title);
        void hideDropPreview();

    //Bookkeeping
        uSize indexOf(const Window* window) const;

        /// @brief Takes the field off the strip. When it was selected, the field now in its place is, or the one before it.
        void removeField(const uSize index);

        /// @brief The colors to draw with: the selected ones for the selected field unless it is disabled, else the state's.
        static const std::vector<glm::vec4>& colorFor(const FieldColors& colors, const ColorState state, const bool selected);

        /// @brief The colors stored for the state.
        static std::vector<glm::vec4>& colorOf(FieldColors& colors, const ColorState state);

        /// @brief Whether any of the colors can be seen, so a fully transparent part isn't drawn at all.
        static bool isVisible(std::span<const glm::vec4> colors);

        static bool contains(const glm::vec4& rect, const glm::vec2& point);
        /// @brief The UTF-8 path of the close icon, in the Resources/img folder next to the executable.
        static std::string getCloseIconPath();

    //Elements
        std::vector<Field> _fields {};
        Background _background { &_projection };
        Border _border { &_projection };
        Box
            _fieldStyle { &_projection }, //drawn once per field, in its colors, so it takes no transitions
            _previewFieldStyle { &_projection }; //the selected look at half its opacity, for the field a dropped window would take
        CloseButton _closeButton { &_projection, ResourceManager::CreateSvgTexture(getCloseIconPath(), &_projection, glm::i32vec2(32)) }; //its own, so its colors and size are this tab's alone.
        std::unique_ptr<Text> _previewTitle { nullptr };
        TransitionValue<glm::vec2> _previewShown {}; //the drop preview field's left edge and width as drawn
        bool _isPreviewShown { false }; //a dropped window's field starts where the preview was

    //Design
        f32
            _stripHeight { 36.0f },
            _minimumFieldWidth { 80.0f },
            _maximumFieldWidth { 240.0f },
            _fieldSpacing { 4.0f },
            _titlePadding { 12.0f },
            _closeButtonSize { 24.0f };
        glm::vec4
            _contentCornerRadii { 0.0f };
        FieldColors
            _titleColors
            {
                ._Enabled = { glm::vec4(0.7f, 0.7f, 0.75f, 1.0f) },
                ._Hover = { glm::vec4(0.9f, 0.9f, 0.95f, 1.0f) },
                ._Clicked = { glm::vec4(1.0f) },
                ._Disabled = { glm::vec4(0.4f, 0.4f, 0.45f, 1.0f) },
                ._Selected = { glm::vec4(1.0f) }
            };
        std::string _titleFontFamily {};
        f32 _titleFontSize { FontManager::ToPixels(18.0f, FontSizeUnit::Points) }; //in pixels, as every stored font size
        PlatformBridge::Fonts::Slant _titleFontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _titleFontWeight { PlatformBridge::Fonts::Weight::Normal };

    //Logic
        static constexpr uSize _kNoField { ~uSize(0) };
        static constexpr f32
            _zStep { 0.01f },
            _closeIconScale { 0.4f },
            _dragThreshold { 5.0f }, //how far a press may move before it drags the field
            _pullOutDistance { 1.5f }, //how far above or below the strip the cursor pulls a field out, in title font sizes
            _pullOutPastEndDistance { 2.2f }; //how far past the first or last field it pulls one out, in title font sizes

        Window
            *_selectedWindow { nullptr },
            *_pressedWindow { nullptr }; //the window whose field the press went to, followed through reordering
        uSize
            _hoveredField { _kNoField },
            _hoveredCloseButton { _kNoField },
            _pressedCloseButton { _kNoField };
        glm::vec2
            _pressPoint { 0.0f }, //where the press began
            _grabOffset { 0.0f }; //where the press holds the field, from its top-left corner with y running downward
        f32
            _dragLeft { 0.0f }, //the dragged field's left edge, which follows the cursor
            _sidewaysTravel { 0.0f }; //how far the drag went sideways at most, which raises the threshold for pulling out
        bool
            _isDraggingField { false },
            _wasPressed { false },
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
