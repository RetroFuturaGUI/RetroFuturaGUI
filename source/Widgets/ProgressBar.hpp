#pragma once
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
    class ProgressBar : public IRangedValue
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

    private:
        void interact();
        bool isInsideGraph(const glm::vec2& mousePos);
        void setColors(const ColorState state);
    };
}
