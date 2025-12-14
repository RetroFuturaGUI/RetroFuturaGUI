#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <string>
#include <vector>

namespace RetroFuturaGUI
{
    struct Grid2dCell
    {
        glm::vec2 _PositionPixels = glm::vec2(0.0f);
        glm::vec2 _PositionNormalized = glm::vec2(0.0f);
        glm::vec2 _SizePixels = glm::vec2(0.0f);
        glm::vec2 _SizeNormalized = glm::vec2(0.0f);
        glm::vec2 _PaddingPixels = glm::vec2(0.0f);
        glm::vec2 _PaddingNormalized = glm::vec2(0.0f);
        u32 _RowSpan = 1;
        u32 _ColSpan = 1;
        std::unique_ptr<IWidget> _Widget = nullptr;
        bool _SpanOccupied = false;
    };

    struct Grid2dAxisDefinition
    {
        std::vector<f32> _RowDefinition;
        std::vector<f32> _ColumnDefinition;
    };

    class Grid2d: public IWidget
    {
    public:
        Grid2d(const IdentityParams& identity, const GeometryParams2D& geometry, const Grid2dAxisDefinition& axisDefinition)
        : IWidget(identity, geometry), _axisdefinition(axisDefinition)
        {
            _grid.reserve(_maxCountPerAxis);

            for(u32 i = 0; i < _maxCountPerAxis; ++i)
            {
                _grid.emplace_back();
                _grid.back().reserve(_maxCountPerAxis);
            }

            for(u32 row = 0; row < _axisdefinition._RowDefinition.size(); ++row)
            {
                for(u32 column = 0; column < _axisdefinition._ColumnDefinition.size(); ++column)
                {
                    f32 posX = 0.0f, posY = 0.0f;

                    for(u32 i = 0; i < row; ++i)
                        posX += _axisdefinition._RowDefinition[i];

                    for(u32 i = 0; i < column; ++i)
                        posY += _axisdefinition._ColumnDefinition[i];

                    _grid[row].push_back(Grid2dCell
                        {
                            glm::vec2(_projection.GetResolution().x * posX, _projection.GetResolution().y * posY),
                            glm::vec2(posX, posY),
                            glm::vec2(_projection.GetResolution().x * _axisdefinition._RowDefinition[row], _projection.GetResolution().y * _axisdefinition._ColumnDefinition[column]),
                            glm::vec2(_axisdefinition._RowDefinition[row], _axisdefinition._RowDefinition[column]),
                            glm::vec2(0.0f),
                            glm::vec2(0.0f),
                            1, 1,
                            nullptr,
                            false
                        }
                    );
                }
            }


            GeometryParams2D geometryb = 
            {
                _projection,
                glm::vec2(0.0f),
                glm::vec2(0.0f),
                0.0f
            };

            BorderParams borderP = 
            {
                glm::vec4(1.0f),
                5.0f
            };

            _debugBorder = std::make_unique<LineBorder>(geometryb, borderP);
        }

        void AttachWidget(u32 row, u32 col, std::unique_ptr<IWidget> widget)
        {
            if(_axisdefinition._RowDefinition.size() < row || _axisdefinition._ColumnDefinition.size() < col)
                return;
                
            if(_grid[row][col]._SpanOccupied)
                return;

            _grid[row][col]._Widget = std::move(widget);
        }

        void Draw() override {};

        void Draw(const bool alsoDrawDebugLines = false)
        {
            for(auto& row : _grid)
            {
                for(auto& cell : row)
                {
                    if(alsoDrawDebugLines)
                        drawDebugLines(cell);

                    if(cell._Widget == nullptr || cell._SpanOccupied)
                        continue;

                    cell._Widget->Draw();
                }
            }


        }

    private:
        void drawDebugLines(const Grid2dCell& cell)
        {
            _debugBorder->Move(glm::vec2(cell._PositionPixels.x + cell._SizePixels.x * 0.5f, _projection.GetResolution().y - (cell._PositionPixels.y + cell._SizePixels.y * 0.5f)));
            _debugBorder->Resize(glm::vec2(cell._SizePixels.x, cell._SizePixels.y));
            _debugBorder->Draw();
        }

        void resizeWidgets()
        {
            for(auto& column : _grid)
            {
                for(auto& cell : column)
                {
                    cell._Widget->SetSize(cell._SizePixels);

                }
            }
        }

        // Data
        std::vector<std::vector<Grid2dCell>> _grid;
        static constinit const u32 _maxCountPerAxis = 64;
        bool _drawDebugLines = false;
        Grid2dAxisDefinition _axisdefinition;
        std::unique_ptr<LineBorder> _debugBorder;
    };
}