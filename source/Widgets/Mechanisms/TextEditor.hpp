#pragma once
#include "IncludeHelper.hpp"
#include "Caret.hpp"
#include "SelectedArea.hpp"
#include "Signal.hpp"
#include "Text.hpp"
#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Whether edited text holds several lines: Up and Down move between them, Enter breaks the line and pasted line breaks stay
    enum class TextLines
    {
        Single,
        Multiple
    };

    /// @brief How the caret shows the character insert mode overwrites: a block behind it, from halfway into the gap before it
    ///        to halfway into the gap after it, or a bar under it. A block needs its owner to draw the caret before the text
    enum class OverwriteCaret
    {
        Block,
        Underline
    };

    /// @brief The world-space area the edited text is visible in. The caret and selection are clipped to it
    struct TextEditArea
    {
        f32
            _Left { 0.0f },
            _Bottom { 0.0f },
            _Right { 0.0f },
            _Top { 0.0f };
    };

    /// @brief What a TextEditor asks of the widget that owns it.
    struct TextEditorHooks
    {
        std::function<Text*()> _GetActiveText {}; //the Text edits apply to, or nullptr while nothing is editable
        std::function<TextEditArea()> _GetTextArea {};
        std::function<bool()> _IsReadOnly {}; //copy and select-all stay available while it returns true
        std::function<void()> _OnChange {}; //an edit changed the text. Emits the widget's own text-change signal
    };

    /// @brief The editing every widget with editable text shares (TextBox, MultilineTextBox, Table): typing, the caret, the
    ///        selection, the clipboard and the keyboard focus. The widget owns it and hands its caret, selection and signals
    ///        out through functions of its own
    class TextEditor final
    {
    public:
        /// @param parentWindow The owning widget's window, whose native window takes the keystrokes
        /// @param hooks Every hook must be set
        /// @param overwriteCaret How the caret shows the character insert mode overwrites
        TextEditor(Projection* projection, GLFWwindow* parentWindow, const TextLines lines, const OverwriteCaret overwriteCaret, TextEditorHooks hooks);
        TextEditor() = delete;
        TextEditor(const TextEditor&) = delete;
        TextEditor(TextEditor&&) = delete;
        auto operator =(const TextEditor&) = delete;
        auto operator =(TextEditor&&) = delete;

        /// @brief Gives up the keystrokes, if this text has them
        ~TextEditor();

        /// @brief Returns the caret for styling: GetCaret().SetColors(...)
        Caret& GetCaret();
        const Caret& GetCaret() const;

        /// @brief Returns the highlight behind the selection for styling: GetSelectedArea().SetColors(...)
        SelectedArea& GetSelectedArea();
        const SelectedArea& GetSelectedArea() const;

        /// @brief Returns the text most recently copied or cut
        const std::string& GetCopiedText() const;

        /// @brief Returns a pointer to the first selected character inside the UTF-8 string, or nullptr when nothing is selected. It
        ///        runs on to the string's end, not to the selection's. GetSelectedString has the selection alone. It stays valid until the text changes
        const char* GetSelection() const;

        /// @brief Returns the selected text (UTF-8). Empty when nothing is selected
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

    //Called by the owning widget
        /// @brief Has the caret's and selection's colors move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

        /// @brief Runs one frame of editing: clipboard, the frame's typed text and Enter, the arrow keys and the caret's blink
        void Interact();

        void DrawSelectedArea();

        /// @brief Draws the caret, unless it is a block behind the text, which DrawCaretBehindText draws
        void DrawCaret();

        /// @brief Draws the caret while it is a block behind the overwritten character. Drawn before the text, so the text covers it
        void DrawCaretBehindText();

        /// @brief Lets typing in, shows the caret and takes the keystrokes
        void BeginEditing();

        /// @brief Stops typing and hides the caret. The keystrokes stay with this text until ReleaseKeyboardFocus
        void EndEditing();

        /// @brief Returns whether typing goes into the text
        bool IsEditing() const;

        /// @brief Puts the caret at the boundary and starts marking from there, as a press into the text does
        void StartMarking(const uSize boundary);

        /// @brief Moves the marked range's end and the caret to the boundary, as a drag does
        void MarkTo(const uSize boundary);

        void StopMarking();
        bool IsMarking() const;

        /// @brief Selects the first characters, as many as given, with the caret after them
        void SelectFromStart(const uSize characterCount);

        /// @brief Drops any selection and stops marking, the marked range back at the text's start
        void ClearSelection();

        /// @brief Puts the caret after the last character and drops the selection, for text set by code rather than typed. No
        ///        keystroke placed them, and the old place may lie past the new text's end
        void PlaceCaretAfterText();

        /// @brief Lays the caret and the selection out again where the text is now, after the widget moved, resized or scrolled
        void FollowText();

        /// @brief Returns whether Enter was pressed in the last editing frame. Its release is signalled in the next
        bool WasEnterPressed() const;

        /// @brief Returns whether typing overwrites the characters after the caret: switched by the Insert key
        bool IsOverwriting() const;

        /// @brief Makes this text the one the keystrokes go to, and captures them on the native window they arrive at. The
        ///        widget's own window, or the host's while that window sits in a tab
        void TakeKeyboardFocus();

        /// @brief Gives up the keystrokes, if this text has them. Another text's keystrokes stay where they are
        void ReleaseKeyboardFocus();

        /// @brief Returns whether the keystrokes go to this text: it took them last, and they still arrive at its window
        bool HasKeyboardFocus() const;

        f32 ClampToTextBounds(const f32 worldX, const f32 halfExtent = 0.0f) const;

    private:
    //Owner hooks
        Text* getActiveText() const;
        TextEditArea getTextArea() const;
        bool isTextReadOnly() const;
        void emitChange();

    //Editing loop
        /// @brief Runs one frame of editing: clipboard, then the frame's typed text and Enter
        void editText();

        /// @brief Steps the caret once per arrow key stroke this frame, the system's repeats included
        void moveCaret();
        void moveCaretLeft();
        void moveCaretRight();

        /// @brief Moves the caret to the closest boundary on the line above, or a step up a column. Single-line text only
        ///        moves in columns
        void moveCaretUp();

        /// @brief Moves the caret to the closest boundary on the line below, or a step down a column. Single-line text only
        ///        moves in columns
        void moveCaretDown();

        void setCaretFromBoundary(const uSize boundary);

        /// @brief Inserts a line break at the caret when Enter is pressed. Single-line text doesn't break
        void insertLineBreak();

        /// @brief Strips what pasted text mustn't bring in. Single-line text turns line breaks into spaces
        void filterPastedText(std::u32string& text) const;

        /// @brief Whether a codepoint must never end up in edited text. Control characters (C0, DEL, C1), surrogate halves and values past the end of Unicode
        static constexpr bool isInvalidCodepoint(const char32_t codepoint)
        {
            return 0x20u > codepoint
                || (0x7Fu <= codepoint && 0x9Fu >= codepoint)
                || (0xD800u <= codepoint && 0xDFFFu >= codepoint)
                || 0x10FFFFu < codepoint;
        }

        
        static constexpr bool isEmojiComponent(const char32_t codepoint)
        {
            return (0xFE00u <= codepoint && 0xFE0Fu >= codepoint)
                || (0xE0100u <= codepoint && 0xE01EFu >= codepoint)
                || (0x1F3FBu <= codepoint && 0x1F3FFu >= codepoint)
                || 0x20E3u == codepoint
                || (0xE0020u <= codepoint && 0xE007Fu >= codepoint);
        }

        /// @brief Whether a code point is half of a flag.
        static constexpr bool isRegionalIndicator(const char32_t codepoint)
        {
            return 0x1F1E6u <= codepoint && 0x1F1FFu >= codepoint;
        }

        /// @brief Where Backspace starts deleting. A whole emoji sequence (its skin tone, presentation selector, keycap, tags and
        ///        whatever a joiner joins), a whole flag, else the one character before the caret, so a diacritic goes on its own
        static uSize getDeletionStart(std::u32string_view text, const uSize caret);

        bool checkForTextCopy();
        bool checkForTextCut();
        bool checkForTextPaste();
        bool checkForSelectAllText();

        /// @brief Emits OnEnterPressed once per press of an Enter key. The line breaks come with the typed text
        void checkForEnterPress();

        /// @brief Applies the frame's typed text in the order it was typed: characters go in, \b deletes like Backspace and \r breaks the line where the text can have several
        void checkForTextInput();

        /// @brief Switches between inserting and overwriting once per press of the Insert key
        void checkForInsertKey();

        /// @brief Lays the caret behind or under the character it would overwrite, or a short way past the line's end where none is
        void moveOverwriteCaret(Text& text, const glm::vec3& caretPosition, const f32 fullHeight);

        /// @brief Whether the caret is a block behind the text now
        bool isCaretBehindText() const;

        /// @brief Draws the caret if it is shown, in view and not blinked off
        void drawShownCaret();

        /// @brief Puts the characters in at the caret' position (over the selection if there is one)
        void insertText(const std::u32string& characters);

        /// @brief Deletes the selection, or else the character before the caret
        void deleteBackward();

        /// @brief Returns how often the key went down this frame, the system's repeats included. None for the key that was down when the focus came, until it is pressed again
        uSize getKeyStrokeCount(const u32 key) const;

        /// @brief Returns how often the Enter keys were pressed, all of them together
        static u32 getEnterPressCount();

        void emitEnterPressed();
        void emitEnterRelease();
        void emitCopy();
        void emitPaste();

        /// @brief Shows the caret and restarts its blink interval, so it stays solid while being moved
        void resetCaretBlink();

        /// @brief Drops any selection and stops mouse marking
        void deselect();

        /// @brief Returns the lower end of the marked range, whichever direction it was dragged in
        uSize getMarkedStart() const;

        /// @brief Returns the upper end of the marked range, whichever direction it was dragged in
        uSize getMarkedEnd() const;

    //Visuals
        /// @brief Flips the caret's visibility once a full blink interval has elapsed
        void updateCaretBlink();

        /// @brief Repositions the caret visual to _caretPosition
        void updateCaretPosition();

        /// @brief Resizes the selection highlight to the marked range
        void updateSelectedArea();

        f32 clampToTextBoundsY(const f32 worldY) const;

        /// @brief Scrolls the edited text sideways so the caret stays inside the text area, and returns the caret's x after scrolling
        f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f);

        /// @brief Returns the caret's full height for the edited text, before updateCaretPosition clips it to the text area
        f32 getCaretHeight() const;

    //Owner
        GLFWwindow* _parentWindow { nullptr };
        TextLines _lines { TextLines::Single };
        OverwriteCaret _overwriteCaret { OverwriteCaret::Underline };
        TextEditorHooks _hooks {};

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
            _enterPressCountSeen { 0 },
            _insertPressCountSeen { 0 };
        bool _isOverwriting { false }; //insert mode: typing replaces the characters after the caret
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
            _caretBlinkState { true };
        std::chrono::high_resolution_clock::time_point _millisecondsPassed { std::chrono::high_resolution_clock::now() };

    //Caret
        Caret _caret;
        static constexpr f32 _caretHeightFactor { 1.6f };
        static constexpr f32 _caretThickness { 2.0f };
        static constexpr f32 _blockCaretDepthOffset { 0.02f }; //behind the text, whose boundaries lie 0.01 in front of it
        bool _isCaretInView { true };

    //Selection
        uSize
            _selectedPositionFirst { 0 },
            _selectedPositionLast { 0 };
        bool
            _isMarking { false },
            _isSelected { false };
        SelectedArea _selectedArea;
        std::vector<SelectedArea::LineArea> _selectedLineAreas {};

        inline static const TextEditor* _keyboardFocus { nullptr }; //the text the keystrokes go to: the one clicked last, in any window
    };
}