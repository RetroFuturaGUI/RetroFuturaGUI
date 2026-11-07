#pragma once
#include "Projection.hpp"
#include "IncludeHelper.hpp"
#include <glm/ext/vector_float4.hpp>
#include <functional>
#include <vector>
#include <span>

namespace RetroFuturaGUI
{
    class IWidgetDecoration
    {
    public:
        IWidgetDecoration(Projection* projection, std::function<void()> onCornerRadiiChange);
        virtual ~IWidgetDecoration() = default;

        virtual void Draw() = 0;
        virtual void SetSize(const glm::vec2& size) = 0;
        virtual void SetPosition(const glm::vec3& position) = 0;
        virtual void SetRotation(const glm::vec3& rotation) = 0;
        virtual glm::vec2 GetSize() const = 0;
        virtual glm::vec3 GetPosition() const = 0;
        virtual glm::vec3 GetRotation() const = 0;

        /// @brief Sets the colors for the given color state.
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Shows the colors of the given state
        void SetColorState(const ColorState state);

        const glm::vec4& GetCornerRadii() const;
        void SetCornerRadii(const glm::vec4& radii);

    protected:
        virtual void setColors() = 0;
        virtual void setCornerRadii() = 0;



        Projection* _projection { nullptr };
        std::function<void()> _onCornerRadiiChange {};
        std::vector<glm::vec4> 
            _colorsEnabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _colorsDisabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _colorsClicked { glm::vec4(0.6f, 0.6f,0.6f, 1.0f) },
            _colorsHover { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) },
            _colorsSelected { };
        glm::vec4 _cornerRadii { 0.0f };

        ColorState _colorState { ColorState::Enabled };
    };
}