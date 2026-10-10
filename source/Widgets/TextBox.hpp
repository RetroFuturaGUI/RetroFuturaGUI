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
        /// @brief Constructs a TextBox widget under the given parent widget/window.
        TextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        TextBox() = delete;
        TextBox(const TextBox&) = delete;
        TextBox(TextBox&&) = delete;
        ~TextBox() = default;
        auto operator =(const TextBox&) = delete;
        auto operator =(TextBox&&) = delete;

    private:
        using ITextProperties::SetText; //setting text directly would bypass the value store - SetValue<std::string_view> renders it
        void renderValueText(std::string_view text, const bool emitSignal) override { ITextProperties::SetText(text, emitSignal); }
        void emitChange() override;
    };
}
