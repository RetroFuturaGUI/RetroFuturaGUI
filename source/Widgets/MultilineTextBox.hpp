#pragma once
#include "IncludeHelper.hpp"
#include "ITextBox.hpp"

namespace RetroFuturaGUI
{
    //A multi-line text input widget
    class MultilineTextBox final : public ITextBox
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

    private:
    //ITextInteraction multiline hooks
        void moveCaretUp() override;
        void moveCaretDown() override;
        void insertLineBreak() override;
        void filterPastedText(std::u32string& text) const override;
    };
}
