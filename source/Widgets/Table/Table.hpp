#pragma once
#include "CheckBox.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "ITextProperties.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "ResourceManager.hpp"
#include "TextBox.hpp"
#include "Label.hpp"
#include "Slider.hpp"
#include "Lasagna.hpp"
#include "config.hpp"
#include "TableText.hpp"
#include "ITextInteraction.hpp"
#include "TableColor.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    class Table final : public IWidget, public IClickable, public IBackground, public IBorder, public ITextInteraction
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

        /// @brief Sets the TableWidget contents in UTF-8.
        void SetTableWidget(std::string_view text, const uSize xIndex, const uSize yIndex, const bool emitSignal);

        /// @brief Sets the TableWidget contents in color.
        void SetTableWidget(const glm::vec4 color, const uSize xIndex, const uSize yIndex, const bool emitSignal);

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

        /// @brief Returns the total size of all tracks laid end to end, which may exceed the table's own size.
        glm::vec2 GetContentExtent() const;

        /// @brief Returns how far the content can scroll on each track before its far edge meets the viewport's; zero when it fits.
        glm::vec2 GetMaxScroll() const;

        /// @brief Returns the text content, in UTF-8.
        const std::string& GetText(const uSize xIndex, const uSize yIndex) const;

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

        /// @brief Sets the caret colour(s). One caret is shared by every cell, so this is table-wide.
        void SetCaretColors(std::span<glm::vec4> colors);

        /// @brief Sets how long, in milliseconds, the caret stays visible/hidden per blink cycle.
        void SetCaretBlinkTime(const f64 milliseconds);

        /// @brief Sets the caret's width and height, in pixels.
        void SetCaretSize(const glm::vec2& size);

        /// @brief Sets the selection highlight colour(s), shared the same way the caret is.
        void SetSelectedAreaColors(std::span<glm::vec4> colors);

        /// @brief Returns the cell currently being edited, or false when no cell has focus.
        bool GetEditedCell(uSize& outRow, uSize& outColumn) const;

        /// @brief Gives up editing focus, hiding the caret and dropping any selection.
        void EndEdit();

        /// @brief Returns the text most recently copied or cut out of a cell.
        const std::string& GetCopiedText() const;

        void Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot);
        void Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot);
        void Disconnect_OnCopy(const typename Signal<>::Slot& slot);
        void Disconnect_OnPaste(const typename Signal<>::Slot& slot);

    private:
        void layoutCells();
        void layoutHeaders();
        void resizeHeaders();
        void interact();

        /// @brief Creates the cell's TableText if it has none yet, styled from the header defaults.
        void ensureHeaderWidget(TableCell& cell);

        /// @brief Returns the top-left world corner of the content viewport, i.e. the table's corner pushed in by any header bands.
        glm::vec2 contentTopLeft() const;

        /// @brief Returns the space each header band takes out of the table: x for the row header's width, y for the column header's height.
        glm::vec2 headerExtents() const;

        /// @brief Draws one header band's cells, with the band's own backgrounds and borders.
        void drawHeaderBand(std::vector<TableCell>& cells, const uSize first, const uSize end);

        struct ScissorState
        {
            bool _WasEnabled { false };
            i32 _Previous[4] { 0, 0, 0, 0 };
        };

        /// @brief Clips to the given rect, intersected with whatever clip is already active, and returns the state to restore.
        static ScissorState pushScissor(const f32 left, const f32 bottom, const f32 width, const f32 height);

        /// @brief Restores the clip state captured by pushScissor.
        static void popScissor(const ScissorState& state);

        /// @brief Resolves each track to a pixel size: Fixed/Auto take their own value, Star tracks divide what the viewport has left.
        static void resolveTrackSizes(const std::vector<TrackDefinition>& tracks, const f32 viewportExtent, std::vector<f32>& outSizes);

        /// @brief Finds the half-open [first, end) range of tracks overlapping the scrolled viewport.
        static void resolveVisibleRange(const std::vector<f32>& sizes, const f32 scroll, const f32 viewportExtent, uSize& outFirst, uSize& outEnd);

        template <typename T> void setWidget(const uSize xIndex, const uSize yIndex, Table* parentTable)
        {
            if(_tableCells.size() <= xIndex)
             return;

            if(_tableCells.front().size() <= yIndex)
                return;

            TableCell& tableCell = _tableCells[xIndex][yIndex];
            tableCell._ParentTable = parentTable;

            if constexpr (std::is_same_v<T, TableText>)
            {
                auto textWidget { std::make_unique<TableText>(parentTable, &_projection) };

                if(_textDefaults._HasFont)
                    textWidget->SetFontFamily(_textDefaults._FontFamily, _textDefaults._FontSize, _textDefaults._Slant, _textDefaults._Weight);

                textWidget->SetTextAlignment(_textDefaults._Alignment);
                textWidget->SetTextPadding(_textDefaults._Padding);

                if(!_nthTrackColors.empty())
                {
                    TrackColoring& coloring { _nthTrackColors[0] };
                    textWidget->SetTextColors(coloring._TextColorEnabled, ColorState::Enabled);
                    textWidget->SetTextColors(coloring._TextColorDisabled, ColorState::Disabled);
                    textWidget->SetTextColors(coloring._TextColorClicked, ColorState::Clicked);
                    textWidget->SetTextColors(coloring._TextColorHover, ColorState::Hover);
                }

                tableCell._TableWidget = std::move(textWidget);
                tableCell._TableWidgetTypeID = ITableWidget::TableWidgetTypeID::TableText;
            }
            else if constexpr (std::is_same_v<T, TableColor>)
            {
                tableCell._TableWidget = std::make_unique<TableColor>(parentTable, &_projection, _cellColorPlane.get());
                tableCell._TableWidgetTypeID = ITableWidget::TableWidgetTypeID::TableColor;
            }

            layoutCells();
        }

        void resizeTrackReadOnlyFlags();

        Text* activeText() const;
        bool isEditedTrackReadOnly() const;
        void beginEdit(const uSize row, const uSize column, const f32 worldX);
        void moveCaret();
        void moveCaretLeft();
        void moveCaretRight();
        void updateCaretPosition();
        void setCaretFromBoundary(const uSize boundary);
        void drawCaretAndSelection();
        void updateSelectedArea();
        void editText();
        bool checkForTextCopy();
        bool checkForTextCut();
        bool checkForTextPaste();
        bool checkForSelectAllText();
        bool checkForKeyRelease();
        bool checkForKeyRepeat();
        bool checkForEnterPress();
        bool checkForBackspacePress();
        bool checkForTextInput();
        void emitEnterPressed();
        void emitEnterRelease();
        void emitCopy();
        void emitPaste();
        void emitChange();

        /// @brief Clamps a world x to the edited cell's horizontal bounds, so a caret or selection edge can't escape its cell.
        f32 clampToCellBounds(const f32 worldX, const f32 halfExtent = 0.0f) const;

        /// @brief Scrolls the edited cell's text just far enough to keep the caret inside the cell, falling back to a hard clamp.
        f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f);

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
        std::vector<std::vector<TableCell>> _tableCells {};
        std::vector<TableCell>    
            _horizontalHeaderCells {},
            _verticalHeaderCells {};
        TextDefaults _textDefaults {};
        std::unique_ptr<Rectangle>
            _highlightedBackgroundCell { nullptr },
            _highlightedCellBorder { nullptr },
            _trackColoringOverlay { nullptr },
            _innerBorder { nullptr },
            _textSelectedArea { nullptr },
            _caret { nullptr },
            _cellColorPlane { nullptr };
        std::vector<ITableWidget::TableWidgetTypeID> _rowWidgetTypes {};

    //Geommetry
        glm::vec3 _innerSize { 0.0f }; // the content viewport: the table's size minus whatever the enabled header bands take out of it
        std::vector<f32>
            _resolvedRowSizes {},
            _resolvedColumnSizes {};
        glm::vec2 _contentExtent { 0.0f };
        glm::vec2 _scrollPosition { 0.0f, 0.0f };

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
            _selectedTextColors {{ 1.0f, 1.0f, 1.0f, 1.0f }};
        TextDefaults _headerTextDefaults {};
        f32
            _innerBorderWidth { 1.0f },
            _headerInnerBorderWidth { 1.0f },
            _headerOuterBorderWidth { 1.0f },
            _horizontalHeaderHeight { 30.0f },
            _verticalHeaderWidth { 60.0f };
        static constexpr f32 _widgetZOffset { 0.05f };

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

    // Text interaction
        uSize
            _editRow { 0 },
            _editColumn { 0 };
        bool _hasEditCell { false };
        std::vector<glm::vec4>
            _caretColors {{ 1.0f, 1.0f, 1.0f, 1.0f }},
            _selectedAreaColors {{ 0.24f, 0.47f, 0.85f, 0.4f }};

        // Caret
        uSize _caretPosition { 0 };
        i32 _caretRepeatDirection { 0 };
        bool _caretKeyWasReleased { true };
        u32 _caretKeyHoldFrames { 0 };

        // Input logic
        bool
            _editingEnabled { false },
            _enterPressed { false },
            _textCopied { false },
            _textCut { false },
            _textPasted { false };
        u32 _keyHoldFrames { 0 };
        std::u32string _keyRepeatText {};
        u32
            _repeatKeySym { 0 },
            _repeatKeyPressCountSeen { 0 },
            _backspaceKeyHoldFrames { 0 },
            _backspacePressCountSeen { 0 };

        // Selection
        std::string _copiedText {};

        Signal<>
            _onTextChange,
            _onTextChangeAsync,
            _onColorChange,
            _onColorChangeAsync,
            _onEnterPressed,
            _onEnterPressedAsync,
            _onEnterReleased,
            _onEnterReleasedAsync,
            _onCopy,
            _onCopyAsync,
            _onPaste,
            _onPasteAsync;
    };
}