#pragma once
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Texture.hpp"
#include "config.hpp"
#include <glm/ext/vector_int2_sized.hpp>
#include <memory>
#include <span>
#include "IRangedValue.hpp"

namespace RetroFuturaGUI
{
    /// @brief A ProgressBar that visualizes a numeric value
    class ProgressBar : public IWidget, public IClickable, public IBackground, public IBorder, public IRangedValue
    {
    public:
        /// @brief Constructs the ProgressBar
        ProgressBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~ProgressBar() = default;
        ProgressBar(const ProgressBar&) = delete;
        ProgressBar(ProgressBar&&) = delete;
        auto operator =(const ProgressBar&) = delete;
        auto operator =(ProgressBar&&) = delete;

        /// @brief Draws the ProgressBar
        void Draw() override;

        /// @brief Sets the ProgressBar's position
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the ProgressBar's size
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the ProgressBar's Rotation
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Sets whether the ProgressBar runs horizontally or vertically at 0° rotation.
        void SetOrientation(const Orientation orientation) override;

        /// @brief Sets the corner rounding radii of the ProgressBar's track background and border.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets the indicator's width and height, in pixels, independently of the track's height.
        void SetIndicatorSize(const glm::vec2& size);

    private:
        void interact();
        bool isInsideGraph(const glm::vec2& mousePos);
        void setColors(const ColorState state);
    };
}
