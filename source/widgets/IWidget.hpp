#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include "Projection.hpp"
#include <span>
#include <string>
#include <string_view>

namespace RetroFuturaGUI
{
    class IWidget
    {
    public:
        IWidget(const IdentityParams& identity, const GeometryParams2D& geometry);
        virtual ~IWidget() = default;
        virtual void Draw() = 0;
        virtual void SetSize(const glm::vec2& size);
        glm::vec2 GetSize() const;
        virtual void SetPosition(const glm::vec2& position);
        glm::vec2 GetPosition() const;
        virtual void SetRotation(const f32 rotation);
        f32 GetRotation() const;
        std::span<glm::vec4> GetBackgroundColors();
        void SetBackgroundColors(std::span<glm::vec4> backgroundColors);
        std::string_view GetName() const;
        void SetName(std::string_view name);
        WidgetTypeID GetWidgetTypeID() const;

        template<typename T> T* GetParentWidget() const
        {
            return static_cast<T*>(_parent);
        }

    protected:
        //identity
        std::string _name;
        void* _parent { nullptr };
        WidgetTypeID 
            _parentTypeID { WidgetTypeID::None },
            _widgetTypeID { WidgetTypeID::None };
        GLFWwindow* _parentWindow { nullptr };

        //geometry
        Projection& _projection;
        glm::vec2 
            _position { 0.0f },
            _size { 0.0f };
        f32 _rotation { 0.0f };

        //style
        std::unique_ptr<glm::vec4[]> _backgroundColors;
        i32 _colorCount { 1 };
        FillType _backgroundColorFillType { FillType::SOLID };
        //float _borderWidth = 2.0f;
        //glm::vec4 _borderColor = {1.0f, 1.0f, 1.0f, 1.0f};
        //glm::vec4 _borderCornerRadius = {0.0f, 0.0f, 0.0f, 0.0f};

        bool isPointInside(const glm::vec2& point) const
        {
            glm::vec2 translatedPoint = point - _position;
            float radians = glm::radians(-_rotation);
            glm::vec2 rotatedPoint(
                translatedPoint.x * cos(radians) - translatedPoint.y * sin(radians),
                translatedPoint.x * sin(radians) + translatedPoint.y * cos(radians)
            );
            return (rotatedPoint.x >= -_size.x * 0.5f &&
                    rotatedPoint.x <= _size.x * 0.5f &&
                    rotatedPoint.y >= -_size.y * 0.5f &&
                    rotatedPoint.y <= _size.y * 0.5f);
        }

    private:

    };
}