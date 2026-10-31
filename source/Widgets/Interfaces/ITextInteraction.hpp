#pragma once
#include "IncludeHelper.hpp"
#include "IWindowAccessor.hpp"
#include "Signal.hpp"
#include "Text.hpp"
#include <chrono>
#include <string>

namespace RetroFuturaGUI
{
    //parts of text interaction that don't care where the text lives
    class ITextInteraction : virtual public IWindowAccessor
    {
    public:
        /// @brief Gives up the keystrokes, if this text has them.
        virtual ~ITextInteraction();

        /// @brief Returns the text most recently copied or cut.
        const std::string& GetCopiedText() const;

        /// @brief Copies the current selection to the clipboard.
        void CopySelectedText();

        /// @brief Cuts the current selection to the clipboard unless the text is read-only.
        void CutSelectedText();

        /// @brief Replaces the selection, or inserts at the caret, with clipboard text unless the text is read-only.
        void PasteSelectedText();

        /// @brief Connects a slot to be called when the Enter key is pressed.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the Enter key is released.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when text is copied.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when text is pasted.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnEnterPressed slot.
        void Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnEnterReleased slot.
        void Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnCopy slot.
        void Disconnect_OnCopy(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnPaste slot.
        void Disconnect_OnPaste(const typename Signal<>::Slot& slot);

    protected:
        /// @brief The Text edits currently apply to, or nullptr when nothing is editable.
        virtual Text* getActiveText() const = 0;

        /// @brief Repositions the caret visual to _caretPosition.
        virtual void updateCaretPosition() = 0;

        /// @brief Resizes the selection highlight to the marked range.
        virtual void updateSelectedArea() = 0;

        /// @brief Emits the host's own text-change signal.
        virtual void emitChange() = 0;

        /// @brief Whether the active text rejects modification. Copy and select-all stay available when true.
        virtual bool isTextReadOnly() const = 0;

    //Editing loop
        /// @brief Runs one frame of editing: clipboard, then the frame's typed text and Enter.
        void editText();

        /// @brief Steps the caret once per arrow key stroke this frame, the system's repeats included.
        void moveCaret();
        void moveCaretLeft();
        void moveCaretRight();
        void setCaretFromBoundary(const uSize boundary);

    //Multiline hooks: single-line text ignores Up/Down and doesn't break lines; multiline widgets override these
        /// @brief Moves the caret to the closest boundary on the line above.
        virtual void moveCaretUp() {}

        /// @brief Moves the caret to the closest boundary on the line below.
        virtual void moveCaretDown() {}

        /// @brief Inserts a line break at the caret when Enter is pressed.
        virtual void insertLineBreak() {}

        /// @brief Strips what pasted text mustn't bring in. Single-line text turns line breaks into spaces.
        virtual void filterPastedText(std::u32string& text) const;

        /// @brief Whether a codepoint must never end up in edited text: control characters (C0, DEL, C1), surrogate halves and values past the end of Unicode.
        static constexpr bool isInvalidCodepoint(const char32_t codepoint)
        {
            return 0x20u > codepoint
                || (0x7Fu <= codepoint && 0x9Fu >= codepoint)
                || (0xD800u <= codepoint && 0xDFFFu >= codepoint)
                || 0x10FFFFu < codepoint;
        }

        bool checkForTextCopy();
        bool checkForTextCut();
        bool checkForTextPaste();
        bool checkForSelectAllText();

        /// @brief Emits OnEnterPressed once per press of an Enter key. The line breaks come with the typed text.
        void checkForEnterPress();

        /// @brief Applies the frame's typed text in the order it was typed: characters go in, \b deletes like Backspace and \r breaks the line where the text can have several.
        void checkForTextInput();

        /// @brief Puts the characters in at the caret' position (over the selection if there is one)
        void insertText(const std::u32string& characters);

        /// @brief Deletes the selection, or else the character before the caret.
        void deleteBackward();

        /// @brief Returns how often the key went down this frame, the system's repeats included. None for the key that was down when the focus came, until it is pressed again.
        uSize getKeyStrokeCount(const u32 key) const;

        /// @brief Returns how often the Enter keys were pressed, all of them together.
        static u32 getEnterPressCount();

        void emitEnterPressed();
        void emitEnterRelease();
        void emitCopy();
        void emitPaste();

        /// @brief Makes this text the one the keystrokes go to, and captures them on the native window they arrive at: the
        ///        widget's own window, or the host's while that window sits in a tab.
        void takeKeyboardFocus();

        /// @brief Gives up the keystrokes, if this text has them. Another text's keystrokes stay where they are.
        void releaseKeyboardFocus();

        /// @brief Returns whether the keystrokes go to this text: it took them last, and they still arrive at its window.
        bool hasInputFocus() const;

        /// @brief Flips the caret's visibility once a full blink interval has elapsed.
        void updateCaretBlink();

        /// @brief Shows the caret and restarts its blink interval, so it stays solid while being moved.
        void resetCaretBlink();

        /// @brief Drops any selection and stops mouse marking.
        void deselect();

        /// @brief Puts the caret after the last character and drops the selection, for text set by code rather than typed: no
        ///        keystroke placed them, and the old place may lie past the new text's end.
        void placeCaretAfterText();

        /// @brief Returns the lower end of the marked range, whichever direction it was dragged in.
        uSize getMarkedStart() const;

        /// @brief Returns the upper end of the marked range, whichever direction it was dragged in.
        uSize getMarkedEnd() const;

    //Caret movement
        uSize _caretPosition { 0 };

    //Input logic
        bool
            _editingEnabled { false },
            _enterPressed { false },
            _textCopied { false },
            _textCut { false },
            _textPasted { false };
        u32
            _focusKeySym { 0 }, //the key down when the focus came
            _focusKeyPressCountSeen { 0 },
            _enterPressCountSeen { 0 };
        std::string _copiedText {};

        Signal<>
            _onEnterPressed,
            _onEnterPressedAsync,
            _onEnterReleased,
            _onEnterReleasedAsync,
            _onCopy,
            _onCopyAsync,
            _onPaste,
            _onPasteAsync;

    //Caret blink
        bool
            _showCaret { false },
            _caretBlinkState { true },
            _caretNeverBlinks { false };
        f64 _blinkForMilliseconds { 650.0 };
        std::chrono::high_resolution_clock::time_point _millisecondsPassed { std::chrono::high_resolution_clock::now() };

    //Selection
        uSize
            _selectedPositionFirst { 0 },
            _selectedPositionLast { 0 };
        bool
            _isMarking { false },
            _isSelected { false };

    private:
        inline static const ITextInteraction* _keyboardFocus { nullptr }; //the text the keystrokes go to: the one clicked last, in any window
    };
}