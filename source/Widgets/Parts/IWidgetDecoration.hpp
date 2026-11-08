#pragma once
#include "Projection.hpp"
#include "IncludeHelper.hpp"
#include "IRectangle.hpp"
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

        /// @brief Sets which corners are cut off straight instead of rounded: SetCutCorners(CutCorners::TopLeft | CutCorners::BottomRight).
        ///        A cut corner is cut by its corner radius along each edge; the others stay rounded, or square with a radius of 0
        void SetCutCorners(const CutCorners corners);
        CutCorners GetCutCorners() const;

    protected:
        virtual void setColors() = 0;
        /// @brief Hands the corner radii and the cut corners to the element
        virtual void setCornerShape() = 0;



        Projection* _projection { nullptr };
        std::function<void()> _onCornerRadiiChange {};
        std::vector<glm::vec4> 
            _colorsEnabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _colorsDisabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _colorsClicked { glm::vec4(0.6f, 0.6f,0.6f, 1.0f) },
            _colorsHover { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) },
            _colorsSelected { };
        glm::vec4 _cornerRadii { 0.0f };
        CutCorners _cutCorners { CutCorners::None };

        ColorState _colorState { ColorState::Enabled };
    };
}