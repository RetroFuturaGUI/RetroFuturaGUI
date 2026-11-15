#pragma once
#include "IncludeHelper.hpp"
#include "ITextBox.hpp"
#include "ITypedText.hpp"

namespace RetroFuturaGUI
{
    //A single-line text input widget that can hold a typed value
    class TextBox final : public ITextBox, public ITypedText
    {
    public:
        /// @brief Constructs a TextBox widget under the given parent widget/window
        TextBox(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        TextBox() = delete;
        TextBox(const TextBox&) = delete;
        TextBox(TextBox&&) = delete;
        ~TextBox() = default;
        auto operator =(const TextBox&) = delete;
        auto operator =(TextBox&&) = delete;

    private:
        void renderValueText(std::string_view text, const bool emitSignal) override;

        /// @brief Marks the text box invalid while its text doesn't read as the type its value is meant to hold
        void valueFitChanged(const bool fits) override;

        void textEdited() override;
        void textChanged(const WidgetTextChange change) override;
    };
}