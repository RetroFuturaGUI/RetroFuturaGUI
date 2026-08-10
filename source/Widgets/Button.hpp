#pragma once
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    //A widget that triggers actions
    class Button : public IWidget, public IClickable, public IBackground, public IBorder, public ITextProperties
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
        void Draw();

        /// @brief Enables or disables the button, optionally emitting the associated signal.
        void SetEnabled(const bool enable, const bool emitSignal = true);

        /// @brief Sets the size of the button.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the button.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the button.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Sets the corner rounding radii of the button's background and border.
        void SetCornerRadii(const glm::vec4& radii);

    private:
        void interact();
        void setColors(const ColorState state);
    };
}