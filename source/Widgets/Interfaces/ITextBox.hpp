#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextEditable.hpp"

namespace RetroFuturaGUI
{
    //Everything TextBox and MultilineTextBox share: the box around the text, mouse interaction and editing.
    class ITextBox : public IWidget, public IClickable, public IBackground, public IBorder, public ITextEditable
    {
    public:
        /// @brief Draws the text box, including its background, border, text, caret and selection highlight.
        void Draw() override;

        /// @brief Enables or disables the text box, optionally emitting the associated signal.
        void SetEnabled(const bool enable, const bool emitSignal = true);

        /// @brief Sets the corner rounding radii of the text box's background and border.
        void SetCornerRadii(const glm::vec4& radii);

    //Geometry
        /// @brief Sets the size of the text box.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the text box.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the text box.
        void SetRotation(const glm::vec3& rotation) override;

    protected:
        /// @brief Creates the box, text, placeholder, caret and selection. Protected: only TextBox and MultilineTextBox create one, and set their own widget type.
        ITextBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);

        void interact();
        void setColors(const ColorState state);
        TextArea textArea() const override final;
    };
}
