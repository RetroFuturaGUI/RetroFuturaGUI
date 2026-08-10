
#pragma once
#include "ITextProperties.hpp"
#include <chrono>

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with editable text capabilities.
    class ITextEditable : virtual public IWindowAccessor, public ITextProperties
    {
    public:
        void SetReadOnly(const bool readOnly);
        bool IsReadOnly() const;
        void SetCaretColors(std::span<glm::vec4> colors);
        void SetCaretFillType(const FillType fillType);
        void SetCaretGradientAnimationSpeed(const f32 speed);
        void SetCaretBlinkTime(const f64 milliseconds);
        void SetSelectedAreaColors(std::span<glm::vec4> colors);
        void SetSelectedAreaFillType(const FillType fillType);
        void SetSelectedAreaGradientAnimationSpeed(const f32 speed);
        void SetSelectedAreaGradientOffset(const f32 gradientOffset);
        void SetSelectedAreaGradientDegree(const f32 degree);
        void SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed);
        void SetSelectedAreaCornerRadii(const glm::vec4& radii);
        void Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot);
        void Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot);
        void Disconnect_OnCopy(const typename Signal<>::Slot& slot);
        void Disconnect_OnPaste(const typename Signal<>::Slot& slot);
        const std::string& GetCopiedText() const;

    protected:
        void moveCaret();
        void editText();
        void updateCaretBlink();
        void drawSelectedArea();
        void updateSelectedArea();
        void setCaretFromBoundary(const uSize boundary);
        void deselect();
        virtual f32 clampToTextBounds(const f32 worldX, const f32 = 0.0f) const { return worldX; }
        virtual f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f) { return clampToTextBounds(worldX, halfExtent); }

        //Caret
        std::unique_ptr<Rectangle> _caret;
        std::vector<glm::vec4> _caretColors { glm::vec4(1.0f) };
        bool
            _showCaret { false },
            _caretBlinkState { true },
            _caretNeverBlinks { false };
        f64 _blinkForMilliseconds { 650.0 };
        std::chrono::high_resolution_clock::time_point _millisecondsPassed { std::chrono::high_resolution_clock::now() };
        uSize _caretPosition { 0 };
        i32 _caretRepeatDirection { 0 };


        bool
            _readOnly { false },
            _editingEnabled { false },
            _keyWasReleased { true },
            _enterPressed { false },
            _textCopied { false },
            _textCut { false },
            _textPasted { false };
        u32 _keyHoldFrames { 0 };
        std::u32string _keyRepeatText {};
        std::vector<char> _prevKeyStates {};
        static constexpr i32 _keyRepeatInitialDelay = 60;
        static constexpr i32 _keyRepeatInterval = 5;

        Signal<>
            _onEnterPressed,
            _onEnterPressedAsync,
            _onEnterReleased,
            _onEnterReleasedAsync,
            _onCopy,
            _onCopyAsync,
            _onPaste,
            _onPasteAsync;

        //Selection
        std::unique_ptr<Rectangle> _selectedArea;
        std::vector<glm::vec4> _selectedAreaColors { glm::vec4(0.24f, 0.47f, 0.85f, 0.4f) };
        uSize
            _selectedPositionFirst { 0 },
            _selectedPositionLast { 0 };
        bool
            _isMarking { false },
            _isSelected { false };
        std::string _copiedText {};

    private:
        void moveCaretLeft();
        void moveCaretRight();
        void emitEnterRelease();
        void emitEnterPressed();
        void emitChange();
        void emitCopy();
        void emitPaste();
        void updateCaretPosition();
        void resetCaretBlink();
        bool checkForTextCopy();
        bool checkForTextCut();
        bool checkForTextPaste();
        bool checkForSelectAllText();
        bool checkForKeyRelease();
        bool checkForKeyRepeat();
        bool checkForEnterPress();
        bool checkForBackspacePress();
        bool checkForTextInput();
    };
}