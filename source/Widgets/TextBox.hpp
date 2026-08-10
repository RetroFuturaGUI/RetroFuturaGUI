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
    class TextBox : public IWidget, public IClickable, public IBackground, public IBorder, public ITextEditable
    {
    public:
        /// @brief Constructs a TextBox widget under the given parent widget/window.
        TextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        TextBox() = delete;
        TextBox(const TextBox&) = delete;
        TextBox(TextBox&&) = delete;
        ~TextBox() = default;
        auto operator =(const TextBox&) = delete;
        auto operator =(TextBox&&) = delete;

        /// @brief Draws the text box, including its background, border, text, caret and selection highlight.
        void Draw() override;

        /// @brief Enables or disables the text box, optionally emitting the associated signal.
        void SetEnabled(const bool enable, const bool emitSignal = true);

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;

        /// @brief Sets the corner rounding radii of the text box's background and border.
        void SetCornerRadii(const glm::vec4& radii);

    //Geometry
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
        f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f) override;

        f32 _scrollOffsetX { 0.0f };
    };
}
