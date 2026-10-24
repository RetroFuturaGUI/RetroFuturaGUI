#pragma once
#include "IncludeHelper.hpp"
#include "ITableWidget.hpp"
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    class ITexture;

    /// @brief Draws a texture it doesn't own into a table cell, as large as fits inside the cell (minding marging and aspect ratio).
    ///        A table has one for all its image cells. The texture and position are set to each cell before drawing.
    class TableImage final : public ITableWidget
    {
    public:
        TableImage() = delete;
        TableImage(Table* parentTable, Projection* projection, Rectangle* rectangle);
        ~TableImage() = default;
        TableImage(const TableImage&) = delete;
        TableImage(TableImage&&) = delete;
        auto operator =(const TableImage&) -> TableImage& = delete;
        auto operator =(TableImage&&) -> TableImage& = delete;

        /// @brief The texture the next Draw shows
        void SetTexture(const ITexture* texture);

        /// @brief The empty space between the picture and the cell's edges.
        void SetCellMargin(const f32 margin);
        void Draw() override;

    private:
        const ITexture* _texture { nullptr };
        Rectangle* _rectangle { nullptr };
        f32 _cellMargin { 4.0f };
    };
}