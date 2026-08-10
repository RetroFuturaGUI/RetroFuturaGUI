#pragma once
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    //A text-based widget that shows information as text
    class Label : public IWidget, public ITextProperties
    {
    public:
        /// @brief Constructs a Label widget under the given parent widget/window.
        Label(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Label() = delete;
        Label(const Label&) = delete;
        Label(Label&&) = delete;
        auto operator =(const Label&) = delete;
        auto operator =(Label&&) = delete;
        ~Label() = default;

        /// @brief Draws the label text.
        void Draw() override;

        /// @brief Enables or disables the label.
        void SetEnabled(const bool enable);

        /// @brief Sets the size of the label.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the label.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the label.
        void SetRotation(const glm::vec3& rotation) override;

    private:
        void setColors(const ColorState state);
    };
}