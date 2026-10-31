#pragma once
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    //A widget that triggers actions
    class Button final : public IWidget, public IClickable, public IBackground, public IBorder, public ITextProperties
    {
    public:
        /// @brief Constructs a Button widget under the given parent widget/window.
        Button(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Button() = delete;
        Button(const Button&) = delete;
        Button(Button&&) = delete;
        auto operator =(const Button&) = delete;
        auto operator =(Button&&) = delete;
        ~Button() = default;

        /// @brief Draws the button, including its background, border and label text.
        void Draw() override;

        /// @brief Sets the corner rounding radii of the button's background and border.
        void SetCornerRadii(const glm::vec4& radii);

    protected:
        /// @brief Sets the size of the button.
        void applySize(const glm::vec3& size) override;

        /// @brief Sets the world position of the button.
        void applyPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the button.
        void applyRotation(const glm::vec3& rotation) override;

    private:
        void interact() override;
        void setColors(const ColorState state) override;
    };
}