#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextEditable.hpp"

namespace RetroFuturaGUI
{
    //A single-line text input widget
    class MultilineTextBox final : public IWidget, public IClickable, public IBackground, public IBorder, public ITextEditable
    {
    public:
        /// @brief Constructs a MultilineTextBox widget under the given parent widget/window.
        MultilineTextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        MultilineTextBox() = delete;
        MultilineTextBox(const MultilineTextBox&) = delete;
        MultilineTextBox(MultilineTextBox&&) = delete;
        ~MultilineTextBox() = default;
        auto operator =(const MultilineTextBox&) = delete;
        auto operator =(MultilineTextBox&&) = delete;

        /// @brief Draws the text box, including its background, border, text, caret and selection highlight.
        void Draw() override;

        /// @brief Enables or disables the text box, optionally emitting the associated signal.
        void SetEnabled(const bool enable, const bool emitSignal = true);

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;

        /// @brief Sets the corner rounding radii of the text box's background and border.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets the size of the text box.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the text box.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the text box.
        void SetRotation(const glm::vec3& rotation) override;

    private:
        void interact();
        void setColors(const ColorState state);
        void drawCaret();
        f32 clampToTextBounds(const f32 worldX, const f32 halfExtent = 0.0f) const override;
        f32 clampToTextBoundsY(const f32 worldY) const override;
        f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f) override;

        f32 _scrollOffsetX { 0.0f };
        std::vector<glm::vec2> _selectedAreas {};
    };
}
