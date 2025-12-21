#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <string>
#include <vector>
#include "LineBorder.hpp"

namespace RetroFuturaGUI
{
    struct Grid2dCell
    {
        glm::vec2
            _PositionPixels { 0.0f },
            _PositionNormalized { 0.0f },
            _SizePixels { 0.0f },
            _SizeNormalized { 0.0f },
            _PaddingPixels { 0.0f },
            _PaddingNormalized { 0.0f };
        u32 
            _RowSpan { 1 },
            _ColSpan { 1 };
        std::unique_ptr<IWidget> _Widget = nullptr;
        bool _SpanOccupied = false;
        SizingMode _SizingMode { SizingMode::FILL };
    };

    struct Grid2dAxisDefinition
    {
        std::vector<f32> _RowDefinition;
        std::vector<f32> _ColumnDefinition;
    };

    class Grid2d: public IWidget
    {
    public:
        Grid2d(const IdentityParams& identity, const GeometryParams2D& geometry, const Grid2dAxisDefinition& axisDefinition);
        void AttachWidget(u32 row, u32 col, std::unique_ptr<IWidget> widget, const SizingMode sizingMode = SizingMode::FILL);
        void Draw() override {};
        void Draw(const bool alsoDrawDebugLines = false);
        void SetSize(const glm::vec2& size);
        void SetPosition(const glm::vec2& position);

    private:
        // Data
        std::vector<std::vector<Grid2dCell>> _grid;
        static constinit const u32 _maxCountPerAxis = 64;
        bool _drawDebugLines = false;
        Grid2dAxisDefinition _axisdefinition;
        std::unique_ptr<LineBorder> _debugBorder;

        void drawDebugLines(const Grid2dCell& cell);
        void resizeCells();
        void resizeWidget(const Grid2dCell& cell);
        void resizeAllWidgets();
        void moveWidgets();
    };
}