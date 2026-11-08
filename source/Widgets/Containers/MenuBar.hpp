#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "Box.hpp"
#include "IClickable.hpp"
#include "Label.hpp"
#include "Button.hpp"
#include "Frame.hpp"
#include "Plane.hpp"
#include "TextBox.hpp"
#include "SvgImage.hpp"
#include "Image.hpp"
#include "CheckBox.hpp"
#include "Slider.hpp"
#include "ProgressBar.hpp"
#include "Prefab.hpp"
#include "ComboBox.hpp"
#include "ColorPreview.hpp"
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    template<typename T>
    concept MenuBarWidgetTypes =
       std::same_as<T, Label>  || std::same_as<T, Button> || std::same_as<T, TextBox>
    || std::same_as<T, Image> || std::same_as<T, SvgImage> || std::same_as<T, CheckBox>
    || std::same_as<T, Slider> || std::same_as<T, ProgressBar> || std::same_as<T, Prefab>
    || std::same_as<T, ComboBox> || std::same_as<T, ColorPreview>;

    class MenuBar final : public IWidget, public IClickable
    {
    public:
        enum class DockingEdge : u32
        {
            Left,
            Right,
            Top,
            Bottom
        };

        enum class EdgeAlignment : u32
        {
            Start,
            Center,
            End
        };

        enum class FlexSizing : u32
        {
            Star,  // takes a share of whatever viewport space the Fixed/Auto tracks left over
            Fixed, // absolute pixel size
            Auto,  // sized to the track's content
            Square // If horizontal alignment: Width equal height. If vertical alignment, height equals width
        };

        struct FlexDefinition
        {
            FlexSizing _FlexSizing { FlexSizing::Fixed };
            f32 _Width { 1.0f };
            bool _IsWindowDrag { false }; //This marks whether a cell can be used to drag the owning window. True is surpressed if the widget is not a Label
            bool _IsWindowUndock { false }; //A drag that starts in this cell takes the owning window out of its docking group first, then drags it alone. True is suppressed if the widget is clickable. The bar disables the cell's widget while the window is docked to nothing
        };

        MenuBar(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        MenuBar() = delete;
        MenuBar(const Button&) = delete;
        MenuBar(MenuBar&&) = delete;
        auto operator =(const MenuBar&) = delete;
        auto operator =(MenuBar&&) = delete;
        ~MenuBar() = default;

        void Draw() override;
        void SetMargin(const f32 margin);
        void SetDockingEdge(const DockingEdge dockingEdge);
        void SetEdgeAlignment(const EdgeAlignment edgeAlignment);
        void SetFlexCount(const uSize count);

        /// @brief Builds a widget of type T in the given flex cell and owns it. The bar is its parent, and passes its projection
        ///        and window on to it.
        /// @param args Constructor arguments after the parent window, such as an SvgImage's path.
        /// @return The new widget, or nullptr, building nothing, when the cell is out of range or taken, or the name is taken.
        ///         The widget lives until it is removed, its cell is dropped, or the bar is destroyed.
        /// @warning Call from the GUI thread only: a widget creates OpenGL objects, and those must stay on the context's thread.
        template<MenuBarWidgetTypes T, typename... Args>
        T* AddWidget(std::string_view name, const uSize index, Args&&... args)
        {
            if(index >= _itemFlex.size())
            {
                std::println("ERROR::MENUBAR::CELL_OUT_OF_RANGE::{}::{}::{}", _name, name, index);
                return nullptr;
            }

            std::unique_ptr<IWidget>& cell { *std::next(_itemFlex.begin(), static_cast<i64>(index)) };

            if(cell)
            {
                std::println("ERROR::MENUBAR::CELL_TAKEN::{}::{}::{}", _name, name, index);
                return nullptr;
            }

            if(isNameTaken(name))
            {
                std::println("ERROR::MENUBAR::WIDGET_NAME_TAKEN::{}::{}", _name, name);
                return nullptr;
            }

            //Most widget constructors take the name as const std::string&, so it is materialised here
            const std::string widgetName { name };
            std::unique_ptr<T> widget { std::make_unique<T>(widgetName, &_projection, this, _widgetTypeID, _parentWindow, std::forward<Args>(args)...) };
            T* addedWidget { widget.get() };
            cell = std::move(widget);
            placeItems();
            return addedWidget;
        }

        /// @brief Returns the widget in the given flex cell, or nullptr when the cell is empty, out of range, or holds a widget of another type.
        template<MenuBarWidgetTypes T> T* GetWidget(const uSize index) const
        {
            if(index >= _itemFlex.size())
                return nullptr;

            return dynamic_cast<T*>(std::next(_itemFlex.begin(), static_cast<i64>(index))->get());
        }

        /// @brief Removes the widget in the given flex cell, which is free again at once. The widget is destroyed at the start of
        ///        the next Interact, so its own slot may remove it.
        /// @return false when the cell is out of range or empty.
        bool RemoveWidget(const uSize index);
        void ShowSeparatorLines(const bool show);
        void SetSeparatorLinesThickness(const f32 thickness);
        /// @brief Sets one sizing policy per flex cell, and resizes the flex to match.
        /// @note Cells beyond the definitions given are dropped, along with the widgets in them, which are destroyed at the start
        ///       of the next Interact. Cells
        ///       added past the previous count start empty. Without a definition a cell is Star-weighted
        ///       at 1, so a flex that was only ever given a count divides the bar evenly.
        void SetFlexDefinition(std::span<FlexDefinition> flexDefinitions);

        /// @brief Returns the sizing policy of the given cell, or the Star-weighted default when out of range.
        const FlexDefinition& GetFlexDefinition(const uSize index) const;

        /// @brief Returns the look every item cell draws behind its widget, per state: Enabled, Hover, Clicked and Disabled
        Box& GetItemStyle();

        /// @brief Returns the look every item cell draws behind its widget, per state: Enabled, Hover, Clicked and Disabled
        const Box& GetItemStyle() const;

        /// @brief Returns the line drawn on every seam between two cells, while ShowSeparatorLines has them shown
        Background& GetSeparatorLine();

        /// @brief Returns the line drawn on every seam between two cells, while ShowSeparatorLines has them shown
        const Background& GetSeparatorLine() const;

        /// @brief Returns the MenuBar's background
        Background& GetBackground();
        
        /// @brief Returns the MenuBar's background
        const Background& GetBackground() const;
        
        /// @brief Returns the MenuBar's border
        Border& GetBorder();
        
        /// @brief Returns the MenuBar's border
        const Border& GetBorder() const;

    protected:
        void applyPosition(const glm::vec3& position) override;

        void applySize(const glm::vec3& size) override;

    private:
        struct MouseState
        {
            glm::vec2 _WorldPoint { 0.0f };   // projection space, y up, what the cells are measured in
            glm::i32vec2 _WindowPoint { 0 };  // window coordinates, y down, what the window is moved in
            bool
                _HasPosition { false },
                _IsPressed { false };
        };

        /// @brief Sweeps the removed items, follows the pointer for the bar, then passes the input on to the items.
        void interact() override;

        /// @brief Passes the frame's update on to every widget, hidden ones included.
        void update() override;

        /// @brief Emits the hover/click signals and works out which flex cell the cursor sits in.
        void followPointer();

        /// @brief Starts, continues or ends dragging the owning window from a cell marked for it.
        void updateWindowDrag(const MouseState& mouse);

        /// @brief Whether the point sits in a cell that may drag the window. Only a Label qualifies:
        ///        anything interactive would have its clicks swallowed by the drag.
        bool isPointInsideDragCell(const glm::vec2& point) const;

        /// @brief Whether the point sits in a cell that pulls the window out of its docking group. Anything that is not
        ///        clickable qualifies, an icon for instance, since the drag would swallow its clicks.
        bool isPointInsideUndockCell(const glm::vec2& point) const;

        /// @brief Takes the window the bar sits in out of its docking group.
        void undockParentWindow() const;

        /// @brief Enables the widgets in undock cells while the window is docked, and disables them while there is nothing to undock it from.
        void syncUndockCells();

        /// @brief Whether the cell holds a disabled widget. Its cell then draws in the disabled colors and takes no presses.
        bool isItemDisabled(const uSize index) const;

        /// @brief Tells the window the bar sits in that the user started dragging it, so it can offer to dock where it is let go.
        void beginParentWindowDrag() const;

        /// @brief Tells the window the bar sits in that the drag ended, which docks it if its preview showed.
        void endParentWindowDrag() const;

        /// @brief Re-docks the bar against its edge and places the items in their cells.
        void updateLayout();

        /// @brief Puts every item in the center of its cell. Cell geometry follows the bar, items keep their own size.
        void placeItems();

        void drawItems();
        void drawSeparatorLines();

        /// @brief Returns the center of the bar along the edge it is docked to, honouring the alignment.
        f32 alignAlongEdge(const f32 edgeLength, const f32 barLength) const;

        /// @brief Returns where the bar sits once its docking edge, margin and alignment are applied.
        glm::vec3 calculateDockedPosition() const;

        /// @brief Whether the bar runs along the X axis, i.e. is docked to the top or the bottom edge.
        bool isHorizontal() const;

        /// @brief Resolves every cell to a size along the bar's length. Fixed, Auto and Square take
        ///        their own extent first, then the Star cells divide whatever the bar has left.
        void resolveCellSizes();

        /// @brief Returns the size of the given flex cell. Zero on both axes when the index is out of range.
        glm::vec2 cellSize(const uSize index) const;

        /// @brief Returns the distance from the bar's starting edge to the given cell's near edge.
        f32 cellOffset(const uSize index) const;

        /// @brief Returns the center of the given flex cell.
        glm::vec3 cellPosition(const uSize index) const;

        /// @brief The state the item in the given cell draws in, given what the cursor is doing.
        ColorState itemState(const uSize index) const;


    //Elements
        Background _background { &_projection };
        Border _border { &_projection };
        Background _separatorLine { &_projection };
        Box _itemStyle { &_projection };
        std::list<std::unique_ptr<IWidget>> _itemFlex {};
        std::vector<std::unique_ptr<IWidget>> _removedWidgets {}; //destroyed at the start of the next Interact

        /// @brief Whether a widget in the flex already has the given name.
        bool isNameTaken(std::string_view name) const;

        /// @brief Keeps the widgets of the cells from count on until the next Interact, before the flex shrinks to count cells.
        void releaseCellsFrom(const uSize count);

    //Design
        f32 _margin { 0.0f }; //margin from window edge
        DockingEdge _dockingEdge { DockingEdge::Top };
        EdgeAlignment _edgeAlignment { EdgeAlignment::Center };
        f32 _separatorLinesThickness { 1.0f };

    //Logic
        bool _useSeparatorLines { false };
        std::vector<FlexDefinition> _flexDefinition {};

        /// @brief Each cell's extent along the bar's length, in pixels, as resolveCellSizes worked it out.
        std::vector<f32> _resolvedCellSizes {};

        /// @brief What a cell without a definition of its own is sized by: an even share of the bar.
        inline static const FlexDefinition _kEvenShare { ._FlexSizing = FlexSizing::Star, ._Width = 1.0f, ._IsWindowDrag = false, ._IsWindowUndock = false };

        /// @brief Stands in for "the cursor is on no cell at all" in _hoveredItemIndex.
        static constexpr uSize _kNoItem { ~uSize(0) };
        uSize _hoveredItemIndex { _kNoItem };

    //Window drag
        bool
            _isDraggingWindow { false },
            _wasDragMousePressed { false };
        glm::i32vec2 _dragGrabPoint { 0 };

    };
}