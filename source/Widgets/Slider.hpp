#pragma once
#include "Texture.hpp"
#include "config.hpp"
#include <glm/ext/vector_int2_sized.hpp>
#include <memory>
#include <span>
#include "IRangedValue.hpp"

namespace RetroFuturaGUI
{
    // A slider which value can be set by dragging its indicator
    class Slider final : public IRangedValue
    {
    public:
    /// @brief Constructs the Slider
        Slider(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~Slider() = default;
        Slider(const Slider&) = delete;
        Slider(Slider&&) = delete;
        auto operator =(const Slider&) = delete;
        auto operator =(Slider&&) = delete;

        /// @brief Draws the Slider
        void Draw() override;

    private:
        void interact();
        bool isInsideIndicator(const glm::vec2& mousePos);
        void setColors(const ColorState state);

    // Settings
        bool _isDraggingIndicator { false };
    };
}
