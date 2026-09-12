#pragma once
#include "IncludeHelper.hpp"
#include "ITableWidget.hpp"
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    class TableColor final : public ITableWidget
    {
    public:
        TableColor() = delete;
        TableColor(Table* parentTable, Projection* projection, Rectangle* rectangle);
        ~TableColor() = default;
        TableColor(const TableColor&) = delete;
        TableColor(TableColor&&) = delete;
        auto operator =(const TableColor&) -> TableColor& = delete;
        auto operator =(TableColor&&) -> TableColor& = delete;
        const glm::vec4& GetColor() const;
        void SetColor(const glm::vec4& color);
        void Draw() override;

    private:
        inline static const glm::vec4 _dummyColor  { 0.5f, 0.0f, 1.0f, 1.0f };
        std::vector<glm::vec4> _color { _dummyColor };
        Rectangle* _rectangle { nullptr };
    };
}