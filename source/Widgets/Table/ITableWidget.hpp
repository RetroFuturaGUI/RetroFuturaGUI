#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    class Table;

    class ITableWidget
    {
    public:
        ITableWidget(Table* parentTable, Projection* projection);
        ITableWidget() = delete;
        virtual ~ITableWidget() = default;

        virtual void Draw() = 0;

        /// @brief Sets the size of the table widget.
        virtual void SetSize(const glm::vec3& size);

        /// @brief Sets the world position of the table widget.
        virtual void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the table widget.
        virtual void SetRotation(const glm::vec3& rotation);

        enum class TableWidgetTypeID : u32
        {
            None,
            TableText,
        };

    protected:
    //Identity
        Table* _parentTable { nullptr };
        TableWidgetTypeID _tableWidgetID { TableWidgetTypeID::None };

    //Geometry
        Projection* _projection { nullptr };
        glm::vec3
            _position { 0.0f },
            _size { 0.0f },
            _rotation { 0.0f };

    //Logic
        bool isPointInside(const glm::vec2& point) const;
        bool isPointInsideRect(const glm::vec2& point, const glm::vec3& size, const glm::vec3& position, const glm::vec3& rotation) const;
    };
}