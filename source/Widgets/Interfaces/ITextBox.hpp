#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "WidgetText.hpp"
#include "TextEditor.hpp"

namespace RetroFuturaGUI
{
    //Everything TextBox and MultilineTextBox share: the box around the text, mouse interaction and editing
    class ITextBox : public IWidget, public IClickable
    {
    public:
        /// @brief Draws the text box, including its background, border, text, caret and selection highlight
        void Draw() final override;

        Background& GetBackground();
        const Background& GetBackground() const;

        /// @brief Takes the keystrokes as a click into the box would, with its whole text selected and the caret at its end, so a dialog opens ready for typing
        void TakeKeyboardFocus();

        /// @brief Returns the TextBox's border
        Border& GetBorder();

        /// @brief Returns the TextBox's border
        const Border& GetBorder() const;

    //Text
        /// @brief Returns the edited text's string, font, colors and effects. Its font, alignment and padding also apply to the placeholder
        WidgetText& GetText();
        const WidgetText& GetText() const;

        /// @brief Returns the text shown while the edited text is empty
        WidgetText& GetPlaceholder();
        const WidgetText& GetPlaceholder() const;

        /// @brief Sets whether the widget rejects text input/editing while still allowing selection and copy
        void EnableReadOnly(const bool enable);

        /// @brief Returns whether the widget is currently read-only
        bool IsReadOnly() const;

    //Editing
        /// @brief Returns the caret for styling: GetCaret().SetColors(...)
        Caret& GetCaret();
        const Caret& GetCaret() const;

        /// @brief Returns the highlight behind the selection for styling: GetSelectedArea().SetColors(...)
        SelectedArea& GetSelectedArea();
        const SelectedArea& GetSelectedArea() const;

        /// @brief Returns the text most recently copied or cut.
        const std::string& GetCopiedText() const;

        /// @brief Returns a pointer to the first selected character inside the UTF-8 string, or nullptr when nothing is selected. It
        ///        runs on to the string's end, not to the selection's. GetSelectedString has the selection alone. It stays valid until the text changes
        const char* GetSelection() const;

        /// @brief Returns the selected text (UTF-8). Empty when nothing is selected.
        std::string GetSelectedString() const;

        /// @brief Copies the current selection to the clipboard
        void CopySelectedText();

        /// @brief Cuts the current selection to the clipboard unless the text is read-only
        void CutSelectedText();

        /// @brief Replaces the selection, or inserts at the caret, with clipboard text unless the text is read-only
        void PasteSelectedText();

        /// @brief Connects a slot to be called when the Enter key is pressed
        /// @param async If true, the slot is invoked asynchronously
        void Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the Enter key is released
        /// @param async If true, the slot is invoked asynchronously
        void Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when text is copied
        /// @param async If true, the slot is invoked asynchronously
        void Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when text is pasted
        /// @param async If true, the slot is invoked asynchronously
        void Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnEnterPressed slot
        void Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnEnterReleased slot
        void Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnCopy slot
        void Disconnect_OnCopy(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnPaste slot
        void Disconnect_OnPaste(const typename Signal<>::Slot& slot);

    protected:
        /// @brief Creates the box, text, placeholder, caret and selection. Protected: only TextBox and MultilineTextBox create one, and set their own widget type
        ITextBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const TextLines lines);

        void interact() override;
        void setColors(const ColorState state) final override;

        /// @brief Called when the text's string, font or padding change, before OnTextChange fires
        virtual void textChanged(const WidgetTextChange change);

        /// @brief Called when typing, cutting or pasting changed the text. Emits OnTextChange
        virtual void textEdited();

        /// @brief Sets the size of the text box.
        void applySize(const glm::vec3& size) final override;

        /// @brief Sets the world position of the text box.
        void applyPosition(const glm::vec3& position) final override;

        TextEditArea getTextArea() const;

        Background _background { &_projection };
        Border _border { &_projection };
        bool _readOnly { false };
        WidgetText _placeholder; //follows _text, so it is declared before it.
        mutable WidgetText _text; //the editor's hook hands it out for editing, also from const functions.
        TextEditor _editor;
    };
}