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
#include <memory>

namespace RetroFuturaGUI
{
    class Table : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
        Table(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Table(const Table&) = delete;
        Table(Table&&) = delete;
        auto operator =(const Table&) = delete;
        auto operator =(Table&&) = delete;
        void SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state) = delete;
        void SetAxisBackgroundColors(std::span<glm::vec4> colors, const ColorState state, const uSize rowNthIndex);
        void SetAxisBorderColors(std::span<glm::vec4> colors, const ColorState state, const uSize axisNthIndex);
        void SetInnerBorderWidth(const f32 width);

        enum class TableOrientation : u32
        {
            Row,
            Column
        };

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

        struct AxisColoring
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
                _SelectedTextColorEnabled {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _SelectedTextColorDisabled {{ 0.5f, 0.5f, 0.5f, 1.0f }},
                _SelectedTextColorClicked {{ 1.0f, 1.0f, 1.0f, 1.0f }},
                _SelectedTextColorHover {{ 1.0f, 1.0f, 1.0f, 1.0f }},
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
        template <typename T> void SetWidget(const uSize xIndex, const uSize yIndex, Table* parentTable)
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

                if(!_nthAxisColors.empty())
                {
                    AxisColoring& coloring { _nthAxisColors[0] };
                    textWidget->SetTextColors(coloring._TextColorEnabled, ColorState::Enabled);
                    textWidget->SetTextColors(coloring._TextColorDisabled, ColorState::Disabled);
                    textWidget->SetTextColors(coloring._TextColorClicked, ColorState::Clicked);
                    textWidget->SetTextColors(coloring._TextColorHover, ColorState::Hover);
                }

                tableCell._TableWidget = std::move(textWidget);
                tableCell._TableWidgetTypeID = ITableWidget::TableWidgetTypeID::TableText;
                layoutCells();
            }
        }

        
        /// @brief Connects a slot to be called when the text content changes.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnTextChange slot.
        void Disconnect_OnTextChange(const typename Signal<>::Slot& slot);

        /// @brief Sets the text color for the given color state.
        void SetTextColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Returns the text color configured for the given color state.
        std::vector<glm::vec4> GetTextColor(const ColorState state) const;

        /// @brief Sets the text contents in UTF-8.
        template<typename T> void SetText(std::string_view text, const uSize xIndex, const uSize yIndex, const bool emitSignal)
        {
            if(_tableCells.size() <= xIndex)
                return;

            if(_tableCells[xIndex].size() <= yIndex)
                return;

            if constexpr (!std::is_same_v<T, TableText>)
                return;

            TableCell& tableCell { _tableCells[xIndex][yIndex] };

            if(!tableCell._TableWidget)
                SetWidget<T>(xIndex, yIndex, this);

            TableText* textWidget { dynamic_cast<TableText*>(tableCell._TableWidget.get()) };
            textWidget->SetText(text);

            if(!emitSignal)
                return;
            
            _onTextChangeAsync.EmitAsync();
            _onTextChange.Emit();
        }

        void SetAxisDefinitions(const std::vector<f32>& rowDefinition, const std::vector<f32>& columnDefinition);

        /// @brief Returns the text content, in UTF-8.
        const std::string& GetText(const uSize xIndex, const uSize yIndex) const;

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        virtual void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Sets the horizontal alignment of the text.
        virtual void SetTextAlignment(const TextAlignment alignment);

        /// @brief Sets the padding applied around the text.
        virtual void SetTextPadding(const f32 padding);

        void SetRowWidgetTypes(const std::vector<ITableWidget::TableWidgetTypeID>& rowWidgetTypes)
        {
            _rowWidgetTypes = rowWidgetTypes;
        }

    private:
        void layoutCells();

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
        TextDefaults _textDefaults {};
        std::unique_ptr<Slider>
            _horizontalScrollbar { nullptr },
            _verticalScrollbar { nullptr };
        std::unique_ptr<Rectangle>
            _highlightedBackgroundCell { nullptr },
            _highlightedCellBorder { nullptr };
        std::vector<ITableWidget::TableWidgetTypeID> _rowWidgetTypes {};
        std::unique_ptr<Rectangle> _axisColoringOverlay { nullptr };
        std::unique_ptr<Rectangle> _innerBorder { nullptr };

    // Design
        std::vector<AxisColoring> _nthAxisColors {};
        uSize _rowColoringVariantCount { 1 };
        static constexpr f32 _widgetZOffset { 0.05f };

    // Logic
        std::vector<f32> 
            _rowDefinition {},
            _columnDefinition {};
        TableOrientation _tableOrientation { TableOrientation::Row };
        bool
            _useHorizontalScrollbar { false },
            _useVerticalScrollbar { false };
        glm::vec2 _scrollPosition { 0.0f, 0.0f };
        uSize
            _displayedRows[2] { 0, 0 },
            _displayedColumns[2] { 0, 0 };    
        Signal<>
            _onTextChange,
            _onTextChangeAsync;
    };
}