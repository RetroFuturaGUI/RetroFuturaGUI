#pragma once
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "ITableWidget.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "Rectangle.hpp"
#include "ResourceManager.hpp"
#include "SvgTexture.hpp"
#include "config.hpp"
#include "TableText.hpp"
#include "ITextEditVisuals.hpp"
#include "TableColor.hpp"
#include "TableCheckBox.hpp"
#include "TableImage.hpp"
#include "TrackIndex.hpp"
#include <array>
#include <chrono>
#include <memory>
#include <limits>
#include <span>
#include <string>
#include "Definitions.hpp"

namespace RetroFuturaGUI
{
    class Table final : public IWidget, public IClickable, public IBackground, public IBorder, public ITextEditVisuals
    {
    public:
        enum class TableOrientation : u32
        {
            Row,
            Column
        };

        enum class TrackSizing : u32
        {
            Star,  // takes a share of whatever viewport space the Fixed/Auto tracks left over
            Fixed, // absolute pixel size
            Auto   // sized to the track's content
        };

        enum class HeaderPosition : u32
        {
            None,
            Top,
            Bottom,
            Left,
            Right
        };

        enum class RowSelection : u32
        {
            None,
            Single,
            Multiple
        };

        /// @brief How an edit of a text cell ended.
        enum class EditEnd : u32
        {
            Kept, //keep changed
            Cancelled //discard changes (esc pressed)
        };

        /// @brief Where a drag of the selected rows is. The table keeps the press until the left mouse button is released
        enum class SelectionDragPhase : u32
        {
            Started, // drag past threshold
            Moved,
            Released, // left mouse button released - the rows are dropped here
            Cancelled // Escape, or the table took the rows away
        };

        /// @brief Sizing policy for one row or column.
        struct TrackDefinition
        {
            TrackSizing _Sizing { TrackSizing::Star };
            f32 _Value { 1.0f }; // Star: a relative weight. Fixed/Auto: pixels.
        };

        Table(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Table(const Table&) = delete;
        Table(Table&&) = delete;
        auto operator =(const Table&) = delete;
        auto operator =(Table&&) = delete;
        void SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state) = delete;
        void SetTrackBackgroundColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex);
        void SetTrackBorderColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex);
        void SetCheckBoxBackgroundColors(std::span<glm::vec4> colors, const ColorState state);
        void SetCheckBoxBorderColors(std::span<glm::vec4> colors, const ColorState state);
        void SetCheckBoxCheckmarkColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the gap/padding between a cell's edge and the checkbox drawn in it, in pixels. The box stays the largest centered square that fits.
        void SetCheckBoxPadding(const f32 padding);

        /// @brief Sets the gap/padding between a checkbox's border and its checkmark in pixels.
        void SetCheckBoxInnerPadding(const f32 padding);
        void SetInnerBorderWidth(const f32 width);
        void SetTableOrientation(const TableOrientation orientation);
        void SetTrackAlternatingColorCount(const uSize variantCount);

        struct TableCell
        {
            std::unique_ptr<ITableWidget> _TableWidget {};
            glm::vec3
                _PositionPixels { 0.0f },
                _PositionNormalized { 0.0f },
                _SizePixels { 0.0f },
                _SizeNormalized { 0.0f },
                _PaddingPixels { 0.0f },
                _PaddingNormalized { 0.0f };
            u32 
                _RowSpan { 1 },
                _ColSpan { 1 };
            bool _SpanOccupied = false;
            SizingMode _SizingMode { SizingMode::FILL };
            ITableWidget::TableWidgetTypeID _TableWidgetTypeID { ITableWidget::TableWidgetTypeID::None };
            Table* _ParentTable { nullptr };
        };

        struct TrackColoring
        {
            std::vector<glm::vec4>
                _BackgroundColorEnabled {{ ResourceManager::_Eigengrau }},
                _BackgroundColorDisabled {{ ResourceManager::_Eigengrau }},
                _BackgroundColorClicked {{ ResourceManager::_Eigengrau }},
                _BackgroundColorHover {{ ResourceManager::_Eigengrau }},
                _InnerBorderColorEnabled {{ 0.4f, 0.4f, 0.4f, 1.0f }},
                _InnerBorderColorDisabled {{ 0.2f, 0.2f, 0.2f, 1.0f }},
                _InnerBorderColorClicked {{ 0.6f, 0.6f, 0.6f, 1.0f }},
                _InnerBorderColorHover {{ 0.5f, 0.5f, 0.5f, 1.0f }},
                _TextColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _TextColorDisabled {{ 0.5f, 0.5f, 0.5f, 1.0f }},
                _TextColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _TextColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _IconColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _IconColorDisabled {{ 0.5f, 0.5f, 0.5f, 1.0f }},
                _IconColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _IconColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _CaretColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _CaretColorDisabled {{ 0.5f, 0.5f, 0.5f, 1.0f }},
                _CaretColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _CaretColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _GraphColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _GraphColorDisabled {{ 0.5f, 0.5f, 0.5f, 1.0f }},
                _GraphColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _GraphColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _IndicatorColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _IndicatorColorDisabled {{ 0.5f, 0.5f, 0.5f, 1.0f }},
                _IndicatorColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _IndicatorColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }};
        };

        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;
                
        /// @brief Connects a slot to be called when the text content changes.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnTextChange slot.
        void Disconnect_OnTextChange(const typename Signal<>::Slot& slot);

        /// @brief Sets the text color for the given color state.
        void SetTextColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex);

        /// @brief Returns the text color configured for the given color state.
        std::vector<glm::vec4> GetTextColor(const ColorState state) const;

        /// @brief Sets the cell's contents in UTF-8. Does not give the cell a value store.
        void SetValue(std::string_view text, const TrackIndex& index, const bool emitSignal);

        /// @brief Sets the cell's contents bool (checkbox)
        void SetValue(const bool value, const TrackIndex& index, const bool emitSignal);

        /// @brief Sets the cell's value, rendered in the cell's numeric base and precision.
        template<NumericValueType T>
        void SetValue(const T value, const TrackIndex& index, const bool emitSignal)
        {
            BodyCell* cell { cellAt(index._Row, index._Column) };

            if(!cell)
                return;

            if(cell->_Type != ITableWidget::TableWidgetTypeID::TableText)
                return;

            if(!cell->_ValueStore)
                cell->_ValueStore = std::make_unique<TextTypes>();

            cell->_ValueStore->SetValue(value);
            cell->_Text = cell->_ValueStore->GetValueText();
            refreshShownText(index._Row, index._Column);

            if(!emitSignal)
                return;

            _onTextChangeAsync.EmitAsync();
            _onTextChange.Emit();
        }

        /// @brief Sets the TableWidget contents in color.
        void SetTableWidget(const glm::vec4 color, const TrackIndex& index, const bool emitSignal);

        /// @brief Makes the cell an image cell showing the texture, fitted inside it; nullptr shows nothing. The table doesn't
        ///        own the texture, which has to outlive the cell's value: set another before destroying it.
        void SetValue(const ITexture* texture, const TrackIndex& index);

        /// @brief The empty space between an image cell's picture and the cell's edges.
        void SetImagePadding(const f32 padding);

        /// @brief The rows in view, from first up to end, which a caller filling image cells lazily fills first.
        void GetDisplayedRows(uSize& firstOut, uSize& endOut) const;

        /// @brief Sets the row/column tracks as Star weights, for callers that just want plain proportions.
        void SetTrackDefinitions(const std::vector<f32>& rowDefinition, const std::vector<f32>& columnDefinition);

        /// @brief Sets the row/column tracks with explicit sizing policies. Fixed/Auto tracks are what let the content outgrow the table and become scrollable.
        void SetTrackDefinitions(const std::vector<TrackDefinition>& rowDefinition, const std::vector<TrackDefinition>& columnDefinition);

        /// @brief Scrolls the content, in pixels from its top-left, clamped to the scrollable extent.
        void SetScrollPosition(const glm::vec2& scrollPosition);

        void SetHorizontalScrollPosition(const f32 scrollPosition);

        void SetVerticalScrollPosition(const f32 scrollPosition);

        glm::vec2 GetScrollPosition() const;

        f32 GetHorizontalScrollPosition() const;

        f32 GetVerticalScrollPosition() const;

        /// @brief Returns the value of the cell at the given index, converted to T.
        template<NumericValueType T>
        T GetValue(const TrackIndex& index) const
        {
            const BodyCell* cell { cellAt(index._Row, index._Column) };

            if(!cell)
                return T {};

            if(cell->_Type != ITableWidget::TableWidgetTypeID::TableText)
                return T {};

            if(!cell->_ValueStore)
                return T {};

            return cell->_ValueStore->GetValue<T>();
        }

        bool GetValue(const TrackIndex& index) const;

        /// @brief Returns the total size of all tracks laid end to end, which may exceed the table's own size.
        glm::vec2 GetContentExtent() const;

        /// @brief Returns how far the content can scroll on each track before its far edge meets the viewport's; zero when it fits.
        glm::vec2 GetMaxScroll() const;

        /// @brief Returns the text content, in UTF-8.
        const std::string& GetText(const TrackIndex& index) const;

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Sets the horizontal alignment of the text.
        void SetTextAlignment(const TextAlignment alignment);

        /// @brief Sets the padding applied around the text.
        void SetTextPadding(const f32 padding);

        void SetRowWidgetTypes(const std::vector<ITableWidget::TableWidgetTypeID>& rowWidgetTypes);

        void ShowHorizontalHeader(const bool show);

        void ShowVerticalHeader(const bool show);

        void SetHorizontalHeaderPosition(const HeaderPosition position);

        void SetVerticalHeaderPosition(const HeaderPosition position);

        void SetHorizontalHeaderTexts(const std::vector<std::string>& texts);

        void SetHorizontalHeaderText(std::string_view text, const uSize columnIndex);

        void SetVerticalHeaderTexts(const std::vector<std::string>& texts);

        void SetVerticalHeaderText(std::string_view text, const uSize rowIndex);

        void SetHeaderFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        void SetHeaderTextAlignment(const TextAlignment alignment);

        void SetHeaderTextPadding(const f32 padding);

        void SetHeaderTextColors(std::span<glm::vec4> colors, const ColorState state);

        void SetHeaderInnerBorderWidth(const f32 width);

        void SetHeaderOuterBorderWidth(const f32 width);

        void SetHeaderInnerBorderColors(std::span<glm::vec4> colors, const ColorState state);

        void SetHeaderOuterBorderColors(std::span<glm::vec4> colors, const ColorState state);

        void SetHeaderBackgroundColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the height of the column header band, in pixels. Taken out of the content viewport.
        void SetHorizontalHeaderSize(const f32 size);

        /// @brief Sets the width of the row header band, in pixels. Taken out of the content viewport.
        void SetVerticalHeaderSize(const f32 size);

    //Text interaction
        void SetTrackReadOnly(const uSize trackIndex, const bool readOnly);

        bool IsTrackReadOnly(const uSize trackIndex) const;

        /// @brief Sets the caret's size, in pixels. Width is fixed at 2 pixels
        void SetCaretSize(const glm::vec2& size);

        /// @brief Returns the cell currently being edited, or false when no cell has focus.
        bool GetEditedCell(uSize& outRow, uSize& outColumn) const;

        /// @brief Gives up editing focus, hiding the caret and dropping any selection. The edit is kept!
        void EndEdit();

        /// @brief Starts editing a text cell from code (on a read-only track as well) and scrolls it into view
        void EditCell(const TrackIndex& index, const uSize selectedLength = std::numeric_limits<uSize>::max());

        /// @brief Connects a slot to an edit of a text cell ending (key press started or when EditCell() was called). The slot gets the cell, how the edit ended, and the cell's text then.
        void Connect_OnCellEditEnded(const typename Signal<TrackIndex, EditEnd, std::string>::Slot& slot, const bool async);

        void Disconnect_OnCellEditEnded(const typename Signal<TrackIndex, EditEnd, std::string>::Slot& slot);

        void SetCheckBoxCornerRadii(const glm::vec4& radii);
        void SetCheckBoxSize(const glm::vec3& size);
        void SetCheckBoxBorderWidth(const f32 width);

    //Row selection
        /// @brief Sets whether a press on a text cell selects its row. In Column orientation it selects the column, since rows
        ///        there are the tracks the colors alternate along. A press on a read-only track then only selects, without a caret.
        ///        Drops the current selection without a signal.
        void SetRowSelection(const RowSelection selection);

        RowSelection GetRowSelection() const;

        /// @brief Returns the selected rows in ascending order. SetTrackDefinitions and SetTableOrientation drop them without a signal.
        std::vector<uSize> GetSelectedRows() const;

        /// @brief Selects the given rows instead of the current ones, skipping rows past the end. Single selection keeps the first. Does nothing while selection is off.
        void SelectRows(std::span<const uSize> rows, const bool emitSignal);

        void ClearSelection(const bool emitSignal);

        void SetSelectedRowBackgroundColors(std::span<glm::vec4> colors);

        void SetSelectedRowTextColors(std::span<glm::vec4> colors);

        /// @brief Marks the row, or the column in Column orientation, that a drag would drop on: its background is drawn in the
        ///        drop target colors, over its own and the selection's. One row at a time; redefining the tracks takes it away.
        void SetDropTargetRow(const uSize row);

        void ClearDropTargetRow();

        void SetDropTargetRowBackgroundColors(std::span<glm::vec4> colors);

        void Connect_OnSelectionChanged(const typename Signal<>::Slot& slot, const bool async);

        void Disconnect_OnSelectionChanged(const typename Signal<>::Slot& slot);

        /// @brief Connects a slot to a second press on the same row within the system's double-click time. The slot gets the row.
        void Connect_OnRowDoubleClicked(const typename Signal<uSize>::Slot& slot, const bool async);

        void Disconnect_OnRowDoubleClicked(const typename Signal<uSize>::Slot& slot);

        /// @brief Connects a slot to a slow double-click: a second press on the same row, the only one selected, that comes too
        ///        late for a double-click but within the slow double-click time, and is let go without a drag. The gesture a file
        ///        list renames with. The slot gets the row.
        void Connect_OnRowSlowDoubleClicked(const typename Signal<uSize>::Slot& slot, const bool async);

        void Disconnect_OnRowSlowDoubleClicked(const typename Signal<uSize>::Slot& slot);

        /// @brief Sets how long after the first press the second one makes a slow double-click: 500 to 1000 ms by default. It
        ///        never starts before the system's double-click time is over, so one pair of presses is never both.
        void SetSlowDoubleClickTime(const std::chrono::milliseconds earliest, const std::chrono::milliseconds latest);

        /// @brief Sets the fill of the band a drag from off the selection draws; translucent, so the rows show through.
        void SetSelectionBandColors(std::span<glm::vec4> colors);

        void SetSelectionBandBorderColors(std::span<glm::vec4> colors);

        /// @brief Connects a slot to the drag of the selected rows: a press on a selected row, or on any row in Single selection,
        ///        that moves past the drag threshold. The slot gets the phase and the pointer in window coordinates.
        void Connect_OnSelectionDrag(const typename Signal<SelectionDragPhase, glm::vec2>::Slot& slot, const bool async);

        void Disconnect_OnSelectionDrag(const typename Signal<SelectionDragPhase, glm::vec2>::Slot& slot);

        /// @brief Finds the row, or the column in Column orientation, under a window space point among those in view.
        bool GetRowAt(const glm::vec2& point, uSize& outRow) const;

    //Keyboard
        /// @brief Makes the arrow keys, Home, End, PageUp and PageDown move the selection, and Ctrl+A select every row, as a
        ///        selecting press does. Shift extends the selection; with Ctrl or Alt held the keys are left to the app.
        void TakeKeyboardFocus();

        /// @brief Returns whether the keys move the selection: the table took them last, and they still arrive at its window.
        bool HasKeyboardFocus() const;

    //Headers and scrolling
        /// @brief Connects a slot to a press on a column header. The slot gets the column.
        void Connect_OnHorizontalHeaderClicked(const typename Signal<uSize>::Slot& slot, const bool async);

        void Disconnect_OnHorizontalHeaderClicked(const typename Signal<uSize>::Slot& slot);

        /// @brief Connects a slot to the table scrolling itself: to keep the row the keys moved to in view, or while a band or a
        ///        drag reaches past the rows. SetScrollPosition doesn't fire it.
        void Connect_OnScrolled(const typename Signal<>::Slot& slot, const bool async);

        void Disconnect_OnScrolled(const typename Signal<>::Slot& slot);

        /// @brief Scrolls just far enough for the row, or the column in Column orientation, to be in view.
        /// @param emitSignal Whether OnScrolled fires when it scrolled.
        void ScrollRowIntoView(const uSize row, const bool emitSignal);


    private:
        struct MouseState
        {
            glm::vec2 _Position { 0.0f };
            bool
                _HasPosition { false },
                _IsInside { false },
                _IsPressed { false },
                _PressedThisFrame { false };
        };

        /// @brief One cell's value. Cells hold no widgets: a text cell in view borrows a TableText from a pool, and checkbox and
        ///        color cells draw through one shared widget each, so a table costs the same however many rows it has.
        struct BodyCell
        {
            std::string _Text {};                      //TableText, in UTF-8
            std::unique_ptr<TextTypes> _ValueStore {}; //a TableText set from a number or edited: what GetValue<T> reads
            glm::vec4 _Color { 0.0f };          //TableColor
            const ITexture* _Texture { nullptr }; //non-ownned TableImage
            ITableWidget::TableWidgetTypeID _Type { ITableWidget::TableWidgetTypeID::None };
            bool
                _IsChecked { false },    //TableCheckBox
                _HasShownText { false }; //a pooled TableText draws it, so a new value has to reach that one too
        };

        /// @brief A pooled TableText drawing a text cell in view, or the edited cell.
        struct ShownText
        {
            uSize
                _Row { 0 },
                _Column { 0 };
            std::unique_ptr<TableText> _Widget { nullptr };
        };

        /// @brief Resolves the tracks' sizes and offsets for the table's size, then lays out the view.
        void layoutCells();

        /// @brief Lays out what scrolling changes: the tracks in view, the pooled texts and the headers.
        void layoutView();
        void layoutHeaders();

        BodyCell* cellAt(const uSize row, const uSize column);
        const BodyCell* cellAt(const uSize row, const uSize column) const;

        /// @brief Returns the cell, of the given type: one that held another type drops what it held, and its TableText.
        BodyCell* cellOfType(const TrackIndex& index, const ITableWidget::TableWidgetTypeID type);

        /// @brief The cell's rect in window space.
        void cellRect(const uSize row, const uSize column, glm::vec3& outPosition, glm::vec3& outSize) const;

        /// @brief Returns the TableText drawing a text cell, taking one from the pool when the cell has none yet.
        TableText* bindText(const uSize row, const uSize column);

        /// @brief Returns the TableText drawing a cell, or nullptr when none does.
        TableText* shownTextAt(const uSize row, const uSize column) const;

        /// @brief Gives a cell's TableText back to the pool.
        void releaseText(const uSize row, const uSize column);

        /// @brief Gives every TableText back to the pool, the edited cell's too.
        void releaseTexts();

        /// @brief Writes a cell's text to the TableText drawing it, if one does.
        void refreshShownText(const uSize row, const uSize column);

        /// @brief Styles a TableText from the table's text defaults and colors, as it joins the pool.
        void styleText(TableText& text);

        /// @brief Styles every pooled TableText again after the defaults or colors changed, laying out the ones in view anew.
        void restyleTexts();

        /// @brief Takes what the edit made of the TableText into its cell, which parses it as its value.
        void syncEditedCell();

        /// @brief Finds the track holding an offset, from the tracks' start offsets followed by the content's end.
        static bool trackAtOffset(const std::vector<f32>& offsets, const f32 offset, uSize& outTrack);
        void resizeHeaders();
        void interact();
        void interactTableText(const MouseState& mouse);
        void interactTableCheckBox(const MouseState& mouse);

        /// @brief Returns whether a press on the text cell would place a caret in it, rather than only select its row.
        bool pressEditsTextCell(const uSize row, const uSize column) const;

        /// @brief Returns whether the mouse is over a text cell a press would edit, where it shows the text bar.
        bool isOverEditableText(const MouseState& mouse) const;

        /// @brief Returns whether the cell overlaps the scrolled viewport.
        bool isCellDisplayed(const uSize row, const uSize column) const;

        /// @brief Returns whether the cell is the one being edited.
        bool isEditCell(const uSize row, const uSize column) const;

        /// @brief Finds the visible cell of the given kind under a world-space point.
        bool findCellAt(const glm::vec2& point, const ITableWidget::TableWidgetTypeID typeID, uSize& outRow, uSize& outColumn) const;

        /// @brief Returns whether a window space point lies on the box of a checkbox cell, which can be smaller than the cell.
        bool isPointOnCheckBox(const glm::vec2& point, const uSize row, const uSize column);

        /// @brief Resolves which color state a checkbox cell draws in, from the hover/press state interact() recorded.
        ColorState checkBoxColorState(const uSize row, const uSize column) const;

        /// @brief Pushes the given state's colors onto the checkbox background/border/checkmark every cell shares.
        void setCheckBoxColors(const ColorState state);

        /// @brief Creates the cell's TableText if it has none yet, styled from the header defaults.
        void ensureHeaderWidget(TableCell& cell);

        /// @brief Returns the top-left world corner of the content viewport, i.e. the table's corner pushed in by any header bands.
        glm::vec2 contentTopLeft() const;

        /// @brief Returns the space each header band takes out of the table: x for the row header's width, y for the column header's height.
        glm::vec2 headerExtents() const;

        /// @brief Draws a text cell through its pooled TableText, in the row's text colors.
        void drawTextCell(const uSize row, const uSize column, std::span<glm::vec4> textColors);

        /// @brief Draws a checkbox cell through the shared checkbox, fitted to the cell.
        void drawCheckBoxCell(const BodyCell& cell, const uSize row, const uSize column, const glm::vec3& position, const glm::vec3& size);

        /// @brief Draws a color cell through the shared color swatch.
        void drawColorCell(const BodyCell& cell, const glm::vec3& position, const glm::vec3& size);
        void drawImageCell(const BodyCell& cell, const glm::vec3& position, const glm::vec3& size);

        /// @brief Draws one header band's cells, with the band's own backgrounds and borders.
        void drawHeaderBand(std::vector<TableCell>& cells, const uSize first, const uSize end);

        struct ScissorState
        {
            bool _WasEnabled { false };
            i32 _Previous[4] { 0, 0, 0, 0 };
        };

        /// @brief Clips to the given rect, intersected with whatever clip is already active, and returns the state to restore.
        ///        The rect is in window pixels; the window's draw origin moves it to where the window lies in the framebuffer.
        ScissorState pushScissor(const f32 left, const f32 bottom, const f32 width, const f32 height) const;

        /// @brief Restores the clip state captured by pushScissor.
        static void popScissor(const ScissorState& state);

        /// @brief Resolves each track to a pixel size: Fixed/Auto take their own value, Star tracks divide what the viewport has left.
        static void resolveTrackSizes(const std::vector<TrackDefinition>& tracks, const f32 viewportExtent, std::vector<f32>& outSizes);

        /// @brief Finds the half-open [first, end) range of tracks overlapping the scrolled viewport, from the tracks' start offsets followed by the content's end.
        static void resolveVisibleRange(const std::vector<f32>& offsets, const f32 scroll, const f32 viewportExtent, uSize& outFirst, uSize& outEnd);

        /// @brief The start offsets of a list of tracks, followed by where the last one ends.
        static void resolveTrackOffsets(const std::vector<f32>& sizes, std::vector<f32>& outOffsets);

        void resizeTrackReadOnlyFlags();

        /// @brief Returns how many rows can be selected: the rows, or the columns in Column orientation.
        uSize selectableRowCount() const;

        /// @brief Drops the selection, its anchor and a pending double-click, without a signal, sized to the current rows.
        void resetSelection();

        /// @brief What a press in the content turned into once it moved.
        enum class PressGesture : u32
        {
            None,
            Pending,      // pressed, not yet past the drag threshold
            Band,         // drawing a selection band
            SelectionDrag // dragging the selected rows
        };

        /// @brief One key the table polls, so a press is taken once and holding it repeats.
        struct KeyRepeat
        {
            u32
                _Key { 0 },
                _PressCountSeen { 0 },
                _HoldFrames { 0 };
        };

        /// @brief Selects from a press on the given row, by the held Alt and Shift, and fires a double-click on a second press there.
        /// @param armsGesture Whether the press may become a band or a drag; not in an edited cell, where a drag marks text.
        void beginRowPress(const uSize row, const glm::vec2& point, const bool armsGesture);

        /// @brief A press on the content where no row is: drops the selection unless Alt is held, and arms a band.
        void beginEmptyPress(const glm::vec2& point);

        /// @brief Follows a press while it is held: past the drag threshold it becomes a band or a drag, and the release ends it.
        void followPress(const MouseState& mouse);

        /// @brief Ends the press gesture without its release, emitting Cancelled for a drag.
        void cancelPress();

        /// @brief Selects the band's rows, added to the selection from before the press while Alt was held.
        void selectBandRows(const glm::vec2& point);

        /// @brief Scrolls while a band or a drag reaches past the rows, faster the farther the pointer is.
        /// @return Whether it scrolled; OnScrolled fired then.
        bool scrollTowardPointer(const glm::vec2& point);
        void drawSelectionBand();

        /// @brief Moves the selection by the keys, while the table has the keyboard and no cell is edited.
        void navigateByKeys();

        /// @brief Moves the selection to a row, extending it from the anchor with Shift, and scrolls it into view.
        void moveCursorTo(const uSize row, const bool extend);

        /// @brief Returns whether the key was pressed since the last call, or is held long enough to repeat.
        static bool isKeyTriggered(KeyRepeat& key, const bool repeats);

        /// @brief Takes the press counts as seen, so keys pressed before the table had the keyboard do nothing.
        void syncKeyCounts();

        /// @brief Finds the column header under a window space point, among the columns in view.
        bool findHorizontalHeaderAt(const glm::vec2& point, uSize& outColumn) const;

        /// @brief Returns whether a window space point lies in the content viewport, the table without its headers.
        bool isPointInContent(const glm::vec2& point) const;

        /// @brief Turns a window space point into content space: from the content's top-left, with the scroll, y running down.
        glm::vec2 toContentPoint(const glm::vec2& point) const;

        /// @brief The start offsets of the selectable tracks, rows or columns in Column orientation, followed by the content's end.
        const std::vector<f32>& selectableTrackOffsets() const;

        /// @brief The offset of a point along the selectable tracks in content space.
        f32 selectableAxisOf(const glm::vec2& contentPoint) const;

        static bool isAltHeld();
        static bool isShiftHeld();
        static bool isControlHeld();

        /// @brief Fires OnSelectionChanged only when the selection differs from the previous one.
        void emitSelectionChange(const std::vector<bool>& previousFlags);

        /// @brief Returns the TableText of the cell currently being edited, or nullptr when no text cell is.
        TableText* activeTableText() const;

        /// @brief Opens a cell for editing: ends another cell's edit, takes the keys and binds the cell's text.
        void openEdit(const uSize row, const uSize column);

        /// @brief Opens a cell for editing from a press, with the caret where it landed.
        void beginEdit(const uSize row, const uSize column, const f32 worldX);

        /// @brief Ends the edit, keeping it or restoring the text it started from, and tells OnCellEditEnded.
        void endEdit(const EditEnd end);

        /// @brief Puts the text from before the edit back into the edited cell.
        void restoreEditedCell();

        /// @brief While a cell is edited, Enter keeps the edit and Escape cancels it.
        void followEditKeys();
        void drawCaretAndSelection();

        //ITextInteraction hooks: edits apply to whichever cell is active, and read-only is per track.
        Text* activeText() const override;
        void emitChange() override;
        bool isTextReadOnly() const override { return isEditedTrackReadOnly(); }

        bool isEditedTrackReadOnly() const;

        /// @brief The edited cell's bounds, so the caret and selection can't escape it
        TextArea textArea() const override;

        struct TextDefaults
        {
            bool _HasFont { false };
            std::string _FontFamily {};
            f32 _FontSize { 16.0f };
            PlatformBridge::Fonts::Slant _Slant { PlatformBridge::Fonts::Slant::Roman };
            PlatformBridge::Fonts::Weight _Weight { PlatformBridge::Fonts::Weight::Normal };
            TextAlignment _Alignment { TextAlignment::Left };
            f32 _Padding { 0.0f };
        };

    //Elements
        std::vector<BodyCell> _cells {}; //row after row, as many per row as there are columns
        std::vector<ShownText> _shownTexts {}; //the pooled texts drawing cells, about as many as fit in the view
        std::vector<std::unique_ptr<TableText>> _freeTexts {}; //the pool's texts not drawing a cell right now
        std::unique_ptr<TableCheckBox> _sharedCheckBox { nullptr }; //draws every checkbox cell, set from each right before
        std::unique_ptr<TableColor> _sharedColor { nullptr };       //draws every color cell the same way
        std::unique_ptr<TableImage> _sharedImage { nullptr };       //draws every image cell, each its own texture
        std::vector<TableCell>
            _horizontalHeaderCells {},
            _verticalHeaderCells {};
        TextDefaults _textDefaults {};
        std::unique_ptr<Rectangle>
            _highlightedBackgroundCell { nullptr },
            _highlightedCellBorder { nullptr },
            _trackColoringOverlay { nullptr },
            _innerBorder { nullptr },
            _cellColorPlane { nullptr },
            _cellImagePlane { nullptr },
            _checkBoxBackground { nullptr },
            _checkBoxBorder { nullptr };
        std::shared_ptr<SvgTexture> _checkmark { nullptr };
        std::vector<glm::vec4> _imagePlaneColors { glm::vec4(0.0f) }; //transparent, so the plane draws just its image; the plane only keeps a view of it
        std::vector<ITableWidget::TableWidgetTypeID> _rowWidgetTypes {};

    //Geommetry
        glm::vec3 _innerSize { 0.0f }; // the content viewport: the table's size minus whatever the enabled header bands take out of it
        std::vector<f32>
            _resolvedRowSizes {},
            _resolvedColumnSizes {},
            _rowOffsets {}, //where each row starts in content space, followed by where the last one ends
            _columnOffsets {}; //the same for the columns
        glm::vec2 _contentExtent { 0.0f };
        glm::vec2 _scrollPosition { 0.0f, 0.0f };
        glm::vec3 _checkBoxSize { _innerSize };

    // Design
        std::vector<TrackColoring> _nthTrackColors {};
        std::vector<glm::vec4>
            _headerBackgroundColorsEnabled {{ ResourceManager::_Eigengrau }},
            _headerBackgroundColorsDisabled {{ ResourceManager::_Eigengrau }},
            _headerBorderColorsEnabled {{ 0.4f, 0.4f, 0.4f, 1.0f }},
            _headerBorderColorsDisabled {{ 0.2f, 0.2f, 0.2f, 1.0f }},
            _headerOuterBorderColorsEnabled {{ 0.4f, 0.4f, 0.4f, 1.0f }},
            _headerOuterBorderColorsDisabled {{ 0.2f, 0.2f, 0.2f, 1.0f }},
            _headerTextColorsEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _headerTextColorsDisabled {{ 0.5f, 0.5f, 0.5f, 1.0f }},
            _selectedTextColors {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBackgroundColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBackgroundColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBackgroundColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBackgroundColorDisabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBorderColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBorderColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBorderColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxBorderColorDisabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxCheckmarkColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxCheckmarkColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxCheckmarkColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _checkBoxCheckmarkColorDisabled {{ 1.0f, 1.0f, 1.0f, 1.0f }};
        TextDefaults _headerTextDefaults {};
        f32
            _innerBorderWidth { 1.0f },
            _headerInnerBorderWidth { 1.0f },
            _headerOuterBorderWidth { 1.0f },
            _horizontalHeaderHeight { 30.0f },
            _verticalHeaderWidth { 60.0f },
            _checkBoxInnerPadding { 5.0f }, // box border -> checkmark
            _checkBoxPadding  { 4.0f },   // cell edge -> box edge
            _imagePadding { 4.0f };         // cell edge -> picture edge
        static constexpr f32 _widgetZOffset { 0.05f };
        SvgPathFill _checkmarkFill
        {
            ._FillType =  FillType::SOLID,
            ._Colors {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            ._GradientDegree = 0.0f,
            ._GradientOffset = 0.0f,
            ._GradientAnimationSpeed = 0.0f,
            ._GradientRotationSpeed = 0.0f
        };

    // Logic
        std::vector<TrackDefinition>
            _rowDefinition {},
            _columnDefinition {};
        bool
            _displayHorizontalHeader { false },
            _displayVerticalHeader { false };
        HeaderPosition
            _horizontalHeaderPosition { HeaderPosition::Top },
            _verticalHeaderPosition { HeaderPosition::Left };
        TableOrientation _tableOrientation { TableOrientation::Row };
        uSize
            _displayedRows[2] { 0, 0 },
            _displayedColumns[2] { 0, 0 };   
        std::vector<bool> _trackReadOnlyFlags {};
        Signal<>
            _onTextChange,
            _onTextChangeAsync,
            _onColorChange,
            _onColorChangeAsync,
            _onCheckBoxChange,
            _onCheckBoxChangeAsync;

    // CheckBox interaction
        uSize
            _hoveredCheckBoxRow { 0 },
            _hoveredCheckBoxColumn { 0 };
        bool
            _hasHoveredCheckBox { false },
            _isHoveredCheckBoxHeld { false }; // the hovered box is being pressed, i.e. it draws in its Clicked colors

    // Text interaction
        uSize
            _editRow { 0 },
            _editColumn { 0 };
        bool
            _hasEditCell { false },
            _isEditFromCode { false }; // EditCell started it, so a read-only track takes the edits
        std::string _editStartText {}; // what Escape puts back
        KeyRepeat
            _editKeepKey { ._Key = PB_KEY_RETURN, ._PressCountSeen = 0, ._HoldFrames = 0 },
            _editCancelKey { ._Key = PB_KEY_ESCAPE, ._PressCountSeen = 0, ._HoldFrames = 0 };
        Signal<TrackIndex, EditEnd, std::string>
            _onCellEditEnded,
            _onCellEditEndedAsync;

    // Row selection
        RowSelection _rowSelection { RowSelection::None };
        std::vector<bool> _selectedRowFlags {}; // one per selectable row
        std::vector<glm::vec4>
            _selectedRowBackgroundColors {{ 0.2f, 0.3f, 0.55f, 1.0f }},
            _selectedRowTextColors {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _dropTargetRowBackgroundColors {{ 0.25f, 0.5f, 0.42f, 1.0f }};
        uSize _dropTargetRow { 0 };
        bool _hasDropTargetRow { false };
        uSize
            _selectionAnchorRow { 0 }, // begin of Shift+click selection
            _lastPressedRow { 0 };
        bool
            _hasSelectionAnchor { false },
            _hasPendingDoubleClick { false }; // the last press can be the first of a double-click
        std::chrono::steady_clock::time_point _lastPressTime {};
        Signal<>
            _onSelectionChanged,
            _onSelectionChangedAsync;
        Signal<uSize>
            _onRowDoubleClicked,
            _onRowDoubleClickedAsync;

    // Press gestures
        PressGesture _pressGesture { PressGesture::None };
        glm::vec2
            _pressPoint { 0.0f }, // window space
            _pressContentPoint { 0.0f }, // content space
            _lastDragPoint { 0.0f };
        uSize _pressedRow { 0 };
        bool
            _canDragSelection { false }, // the press landed on a row selected before it, or on any row in Single selection
            _collapseOnRelease { false }, // a plain press on a selected row selects only that row once released without a drag
            _isSlowDoubleClickPending { false }; // the press is a slow double-click's second, if it is let go without a drag
        std::chrono::milliseconds
            _slowDoubleClickEarliest { 500 },
            _slowDoubleClickLatest { 1000 };
        Signal<uSize>
            _onRowSlowDoubleClicked,
            _onRowSlowDoubleClickedAsync;
        std::vector<bool> _bandBaseFlags {}; // what the band adds to: the selection from before the press with Alt, nothing without
        std::unique_ptr<Rectangle>
            _selectionBand { nullptr },
            _selectionBandBorder { nullptr };
        std::vector<glm::vec4>
            _selectionBandColors {{ 0.3f, 0.45f, 0.85f, 0.25f }},
            _selectionBandBorderColors {{ 0.45f, 0.6f, 1.0f, 0.9f }};
        std::chrono::steady_clock::time_point _lastAutoScrollTime {};
        Signal<SelectionDragPhase, glm::vec2>
            _onSelectionDrag,
            _onSelectionDragAsync;
        static constexpr f32
            _dragThreshold { 5.0f }, // how far a press may move before it's a drag
            _autoScrollPixelsPerSecond { 15.0f },
            _autoScrollMaxPixelsPerSecond { 3000.0f };

    // Keyboard
        uSize _cursorRow { 0 }; // the row the keys move from
        bool
            _hasCursorRow { false },
            _hadKeyboardFocus { false };
        std::array<KeyRepeat, 6> _navigationKeys
        {{
            { ._Key = PB_KEY_UP, ._PressCountSeen = 0, ._HoldFrames = 0 },
            { ._Key = PB_KEY_DOWN, ._PressCountSeen = 0, ._HoldFrames = 0 },
            { ._Key = PB_KEY_PAGE_UP, ._PressCountSeen = 0, ._HoldFrames = 0 },
            { ._Key = PB_KEY_PAGE_DOWN, ._PressCountSeen = 0, ._HoldFrames = 0 },
            { ._Key = PB_KEY_HOME, ._PressCountSeen = 0, ._HoldFrames = 0 },
            { ._Key = PB_KEY_END, ._PressCountSeen = 0, ._HoldFrames = 0 }
        }};
        KeyRepeat
            _selectAllKey { ._Key = PB_KEY_A, ._PressCountSeen = 0, ._HoldFrames = 0 },
            _escapeKey { ._Key = PB_KEY_ESCAPE, ._PressCountSeen = 0, ._HoldFrames = 0 };

    // Headers and scrolling
        Signal<uSize>
            _onHorizontalHeaderClicked,
            _onHorizontalHeaderClickedAsync;
        Signal<>
            _onScrolled,
            _onScrolledAsync;
    };
}