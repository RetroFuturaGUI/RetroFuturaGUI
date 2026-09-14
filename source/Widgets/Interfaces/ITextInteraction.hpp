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
        /// @brief Returns the text most recently copied or cut.
        const std::string& GetCopiedText() const;

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
        virtual Text* activeText() const = 0;

        /// @brief Repositions the caret visual to _caretPosition.
        virtual void updateCaretPosition() = 0;

        /// @brief Resizes the selection highlight to the marked range.
        virtual void updateSelectedArea() = 0;

        /// @brief Emits the host's own text-change signal.
        virtual void emitChange() = 0;

        /// @brief Whether the active text rejects modification. Copy and select-all stay available when true.
        virtual bool isTextReadOnly() const = 0;

    //Editing loop
        /// @brief Runs one frame of editing: clipboard, then key repeat, Enter, backspace and character input.
        void editText();

        void moveCaret();
        void moveCaretLeft();
        void moveCaretRight();
        void setCaretFromBoundary(const uSize boundary);

        bool checkForTextCopy();
        bool checkForTextCut();
        bool checkForTextPaste();
        bool checkForSelectAllText();
        bool checkForKeyRelease();
        bool checkForKeyRepeat();
        bool checkForEnterPress();
        bool checkForBackspacePress();
        bool checkForTextInput();

        void emitEnterPressed();
        void emitEnterRelease();
        void emitCopy();
        void emitPaste();

    //Caret movement
        uSize _caretPosition { 0 };
        i32 _caretRepeatDirection { 0 };
        bool _caretKeyWasReleased { true };
        u32 _caretKeyHoldFrames { 0 };

    //Input logic
        bool
            _editingEnabled { false },
            _enterPressed { false },
            _textCopied { false },
            _textCut { false },
            _textPasted { false };
        u32 _keyHoldFrames { 0 };
        std::u32string _keyRepeatText {};
        u32
            _repeatKeySym { 0 },
            _repeatKeyPressCountSeen { 0 },
            _backspaceKeyHoldFrames { 0 },
            _backspacePressCountSeen { 0 };
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

        /// @brief Returns whether the widget's window is the one currently taking keyboard input.
        bool hasInputFocus() const;

        /// @brief Flips the caret's visibility once a full blink interval has elapsed.
        void updateCaretBlink();

        /// @brief Shows the caret and restarts its blink interval, so it stays solid while being moved.
        void resetCaretBlink();

        /// @brief Drops any selection and stops mouse marking.
        void deselect();

        /// @brief Returns the lower end of the marked range, whichever direction it was dragged in.
        uSize markedStart() const;

        /// @brief Returns the upper end of the marked range, whichever direction it was dragged in.
        uSize markedEnd() const;

        static bool shouldRepeat(const u32 holdFrames);

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

    //Key repeat cadence
        static constexpr u32
            _keyRepeatInitialDelay { 60 },
            _keyRepeatInterval { 5 };
    };
}
