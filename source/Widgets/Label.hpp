#pragma once
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "WidgetText.hpp"

namespace RetroFuturaGUI
{
    //A text-based widget that shows information as text
    class Label final : public IWidget
    {
    public:
        /// @brief Constructs a Label widget under the given parent widget/window
        Label(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Label() = delete;
        Label(const Label&) = delete;
        Label(Label&&) = delete;
        auto operator =(const Label&) = delete;
        auto operator =(Label&&) = delete;
        ~Label() = default;

        /// @brief Draws the label text.
        void Draw() override;

        /// @brief Returns the widget's text Its string, font, colors and effects
        WidgetText& GetText();
        const WidgetText& GetText() const;

    protected:
        /// @brief Sets the size of the label.
        void applySize(const glm::vec3& size) override;

        /// @brief Sets the world position of the label.
        void applyPosition(const glm::vec3& position) override;

    private:
        void setColors(const ColorState state) override;

        WidgetText _text { &_projection };
    };
}