#pragma once
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    //A widget that triggers actions
    class Button final : public IWidget, public IClickable, public ITextProperties
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

        /// @brief Returns the button's background
        Background& GetBackground();
        const Background& GetBackground() const;

        /// @brief Returns the button's border
        Border& GetBorder();
        const Border& GetBorder() const;

    protected:
        /// @brief Sets the size of the button.
        void applySize(const glm::vec3& size) override;

        /// @brief Sets the world position of the button.
        void applyPosition(const glm::vec3& position) override;

    private:
        void interact() override;
        void setColors(const ColorState state) override;

        Background _background { &_projection };
        Border _border { &_projection };
    };
}