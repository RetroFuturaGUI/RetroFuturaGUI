#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "Label.hpp"
#include "Button.hpp"
#include "Rectangle.hpp"
#include "TextBox.hpp"
#include "SvgImage.hpp"
#include "Image.hpp"
#include "CheckBox.hpp"
#include "Slider.hpp"
#include "ProgressBar.hpp"
#include "Prefab.hpp"
#include "ComboBox.hpp"
#include <memory>
#include <vector>

namespace RetroFuturaGUI
{
    template<typename T>
    concept MenuBarWidgetTypes =
       std::same_as<T, Label>  || std::same_as<T, Button> || std::same_as<T, TextBox>
    || std::same_as<T, Image> || std::same_as<T, SvgImage> || std::same_as<T, CheckBox>
    || std::same_as<T, Slider> || std::same_as<T, ProgressBar> || std::same_as<T, Prefab>
    || std::same_as<T, ComboBox>;

    class MenuBar final : public IWidget, public IBackground, public IBorder, public IClickable
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
        };

        MenuBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        MenuBar() = delete;
        MenuBar(const Button&) = delete;
        MenuBar(MenuBar&&) = delete;
        auto operator =(const MenuBar&) = delete;
        auto operator =(MenuBar&&) = delete;
        ~MenuBar() = default;

        void Draw() override;
        void SetPosition(const glm::vec3& position) override;
        void SetSize(const glm::vec3& size) override;
        void SetRotation(const glm::vec3& rotation) override;
        void SetMargin(const f32 margin);
        void SetDockingEdge(const DockingEdge dockingEdge);
        void SetEdgeAlignment(const EdgeAlignment edgeAlignment);
        void SetFlexCount(const uSize count);

        /// @brief Takes ownership of a widget and puts it in the given flex cell
        template<MenuBarWidgetTypes T> void AddWidget(std::unique_ptr<T> widget, const uSize index)
        {
            if(!widget)
                return;

            if(index >= _itemFlex.size())
                return;

            *std::next(_itemFlex.begin(), static_cast<i64>(index)) = std::move(widget);
            placeItems();
        }

        /// @brief Returns the widget in the given flex cell, or nullptr when the cell is empty, out of range, or holds a widget of another type.
        template<MenuBarWidgetTypes T> T* GetWidget(const uSize index) const
        {
            if(index >= _itemFlex.size())
                return nullptr;

            return dynamic_cast<T*>(std::next(_itemFlex.begin(), static_cast<i64>(index))->get());
        }

        bool RemoveWidget(const uSize index);
        void EnableSeparatorLines(const bool enable);
        void SetSeparatorLinesThickness(const f32 thickness);
        /// @brief Sets one sizing policy per flex cell, and resizes the flex to match.
        /// @note Cells beyond the definitions given are dropped, along with the widgets in them. Cells
        ///       added past the previous count start empty. Without a definition a cell is Star-weighted
        ///       at 1, so a flex that was only ever given a count divides the bar evenly.
        void SetFlexDefinition(std::span<FlexDefinition> flexDefinitions);

        /// @brief Returns the sizing policy of the given cell, or the Star-weighted default when out of range.
        const FlexDefinition& GetFlexDefinition(const uSize index) const;

    //Separator lines
        /// @brief Sets a single separator line color for the given color state.
        void SetSeparatorLinesColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple separator line colors (e.g. gradient stops) for the given color state.
        void SetSeparatorLinesColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the separator lines' fill type (solid, linear/radial/huestar gradient).
        void SetSeparatorLinesFillType(const FillType fillType);

        /// @brief Sets the offset applied to the separator lines' gradient start position.
        void SetSeparatorLinesGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the separator lines' gradient animates over time.
        void SetSeparatorLinesGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the separator lines' linear gradient, in degrees.
        void SetSeparatorLinesGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the separator lines' gradient rotates over time.
        void SetSeparatorLinesGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets sections of the separator lines to skip drawing, which is what turns them into dashed lines; see Rectangle::SetBackgroundGaps.
        void SetSeparatorLinesGaps(const BackgroundGap& gap);

    //Item background
        /// @brief Sets a single item background color for the given color state.
        void SetItemBackgroundColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple item background colors (e.g. gradient stops) for the given color state.
        void SetItemBackgroundColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the corner rounding radii of the item background and item border.
        void SetItemCornerRadii(const glm::vec4& radii);

        /// @brief Sets the item background fill type (solid, linear/radial/huestar gradient).
        void SetItemBackgroundFillType(const FillType fillType);

        /// @brief Sets the offset applied to the item background gradient's start position.
        void SetItemBackgroundGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the item background gradient animates over time.
        void SetItemBackgroundGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the item background's linear gradient, in degrees.
        void SetItemBackgroundGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the item background gradient rotates over time.
        void SetItemBackgroundGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the item background's glass-effect-with-image shader feature.
        void SetItemWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the item background's Dotted raster pattern.
        void SetItemBackgroundPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the item background's Dotted raster pattern.
        void SetItemBackgroundDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the item background's dot widths/animation transfer.
        void SetItemBackgroundDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels (each value is a dot's full width across), for the item background's Dotted raster pattern. Enables the Raster shader feature when non-empty.
        void SetItemBackgroundPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each item background dot's opacity reaches from its center before fading to transparent.
        void SetItemBackgroundDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the item background's dotted pattern animates.
        void SetItemBackgroundDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the item background's FogEffect shader feature.
        void SetItemBackgroundFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the item background's fog drifts over time.
        void SetItemBackgroundFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the item background's fog. Enables the FogEffect shader feature when non-empty.
        void SetItemBackgroundFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the item background's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetItemBackgroundFogClearing(const f32 clearing);

        /// @brief Sets sections of the item background to skip drawing; see Rectangle::SetBackgroundGaps.
        void SetItemBackgroundGaps(const BackgroundGap& gap);

    //Item border
        /// @brief Sets a single item border color for the given color state.
        void SetItemBorderColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple item border colors (e.g. gradient stops) for the given color state.
        void SetItemBorderColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the item border fill type (solid, linear/radial/huestar gradient).
        void SetItemBorderFillType(const FillType fillType);

        /// @brief Sets the offset applied to the item border gradient's start position.
        void SetItemBorderGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the item border gradient animates over time.
        void SetItemBorderGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the item border's linear gradient, in degrees.
        void SetItemBorderGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the item border gradient rotates over time.
        void SetItemBorderGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the item border's glass-effect-with-image shader feature.
        void SetItemWindowBorderImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the item border's Dotted raster pattern.
        void SetItemBorderPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the item border's Dotted raster pattern.
        void SetItemBorderDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the item border's dot widths/animation transfer.
        void SetItemBorderDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels (each value is a dot's full width across), for the item border's Dotted raster pattern. Enables the Raster shader feature when non-empty.
        void SetItemBorderPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each item border dot's opacity reaches from its center before fading to transparent.
        void SetItemBorderDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the item border's dotted pattern animates.
        void SetItemBorderDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the item border's width.
        void SetItemBorderWidth(const f32 borderWidth);

        /// @brief Sets sections of the item border to skip drawing; see Rectangle::SetBorderGaps.
        void SetItemBorderGaps(std::span<BorderGap> gaps);



    private:
        struct MouseState
        {
            glm::vec2 _WorldPoint { 0.0f };   // projection space, y up, what the cells are measured in
            glm::i32vec2 _WindowPoint { 0 };  // window coordinates, y down, what the window is moved in
            bool
                _HasPosition { false },
                _IsPressed { false };
        };

        /// @brief Emits the hover/click signals and works out which flex cell the cursor sits in.
        void interact();

        /// @brief Starts, continues or ends dragging the owning window from a cell marked for it.
        void updateWindowDrag(const MouseState& mouse);

        /// @brief Whether the point sits in a cell that may drag the window. Only a Label qualifies:
        ///        anything interactive would have its clicks swallowed by the drag.
        bool isPointInsideDragCell(const glm::vec2& point) const;

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

        /// @brief Makes the given state the one the items draw in, and pushes the matching colors.
        void setItemColors(const ColorState state);

        /// @brief Hands the colors to the element, unless the state they came from was never set.
        void pushElementColors(Rectangle* element, std::vector<glm::vec4>* colors);

        void setSeparatorLinesColors();
        void setItemBackgroundColors();
        void setItemBorderColors();
        void setItemBackgroundCornerRadii(const glm::vec4& radii);
        void setItemBorderCornerRadii(const glm::vec4& radii);

    //Elements
        std::unique_ptr<Rectangle>
            _separatorLine { nullptr },
            _itemBackground { nullptr },
            _itemBorder { nullptr };
        std::list<std::unique_ptr<IWidget>> _itemFlex {};

    //Design
        f32 _margin { 0.0f }; //margin from window edge
        DockingEdge _dockingEdge { DockingEdge::Top };
        EdgeAlignment _edgeAlignment { EdgeAlignment::Center };
        std::vector<glm::vec4>
            _separatorLinesColorsEnabled {},
            _separatorLinesColorsDisabled {},
            _itemBackgroundColorsEnabled {},
            _itemBackgroundColorsDisabled {},
            _itemBackgroundColorsHover {},
            _itemBackgroundColorsClick {},
            _itemBorderColorsEnabled {},
            _itemBorderColorsDisabled {},
            _itemBorderColorsHover {},
            _itemBorderColorsClick {};
        std::vector<f32>
            _itemBackgroundPrimaryRasterWidthTransfer {},
            _itemBackgroundFogDensity {},
            _itemBorderPrimaryRasterWidthTransfer {};

        f32 _separatorLinesThickness { 1.0f };

    //Logic
        ColorState
            _separatorLinesColorState { ColorState::Enabled },
            _itemBackgroundColorState { ColorState::Enabled },
            _itemBorderColorState { ColorState::Enabled };
        bool _useSeparatorLines { false };
        std::vector<FlexDefinition> _flexDefinition {};

        /// @brief Each cell's extent along the bar's length, in pixels, as resolveCellSizes worked it out.
        std::vector<f32> _resolvedCellSizes {};

        /// @brief What a cell without a definition of its own is sized by: an even share of the bar.
        inline static const FlexDefinition _kEvenShare { ._FlexSizing = FlexSizing::Star, ._Width = 1.0f, ._IsWindowDrag = false };

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