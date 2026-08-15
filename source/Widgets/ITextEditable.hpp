
#pragma once
#include "ITextProperties.hpp"
#include <chrono>
#include <memory>

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with editable text capabilities.
    class ITextEditable : virtual public IWindowAccessor, public ITextProperties
    {
    public:
        /// @brief Sets whether the widget rejects text input/editing while still allowing selection and copy.
        void SetReadOnly(const bool readOnly);

        /// @brief Returns whether the widget is currently read-only.
        bool IsReadOnly() const;

        /// @brief Sets the caret color(s).
        void SetCaretColors(std::span<glm::vec4> colors);

        /// @brief Sets the caret fill type (solid, linear/radial/huestar gradient).
        void SetCaretFillType(const FillType fillType);

        /// @brief Sets the speed at which the caret's gradient animates over time.
        void SetCaretGradientAnimationSpeed(const f32 speed);

        /// @brief Sets how long, in milliseconds, the caret stays visible/hidden per blink cycle.
        void SetCaretBlinkTime(const f64 milliseconds);

        /// @brief Sets the selection highlight color(s).
        void SetSelectedAreaColors(std::span<glm::vec4> colors);

        /// @brief Sets the selection highlight fill type (solid, linear/radial/huestar gradient).
        void SetSelectedAreaFillType(const FillType fillType);

        /// @brief Sets the speed at which the selection highlight's gradient animates over time.
        void SetSelectedAreaGradientAnimationSpeed(const f32 speed);

        /// @brief Sets the offset applied to the selection highlight gradient's start position.
        void SetSelectedAreaGradientOffset(const f32 gradientOffset);

        /// @brief Sets the angle of the selection highlight's linear gradient, in degrees.
        void SetSelectedAreaGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the selection highlight's gradient rotates over time.
        void SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the corner rounding radii of the selection highlight.
        void SetSelectedAreaCornerRadii(const glm::vec4& radii);

        /// @brief Connects a slot to be called when the Enter key is pressed.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the Enter key is released.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when text is copied from the widget.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when text is pasted into the widget.
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

        /// @brief Returns the text most recently copied or cut from the widget.
        const std::string& GetCopiedText() const;

        /// @brief Sets the placeholder text color for the given color state.
        void SetPlaceholderTextColor(const glm::vec4& color);

        /// @brief Sets the placeholder text content, in UTF-8.
        void SetPlaceholderText(std::string_view text);

        /// @brief Returns the placeholder text content, in UTF-8.
        const std::string& GetPlaceholderText() const;

        /// @brief Sets the font family, size and style used to render the text and placeholder text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;

        /// @brief Sets the horizontal alignment of the text and placeholder text.
        void SetTextAlignment(const TextAlignment alignment) override;

        /// @brief Sets the padding applied around the text and placeholder text.
        void SetTextPadding(const f32 padding) override;


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
        bool _caretKeyWasReleased { true };
        u32 _caretKeyHoldFrames { 0 };

        //input logic
        bool
            _readOnly { false },
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
        std::vector<char> _prevKeyStates {};
        static constexpr i32 _keyRepeatInitialDelay { 60 };
        static constexpr i32 _keyRepeatInterval { 5 };

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

        //Placeholder Text
        std::unique_ptr<Text> _placeholderText { nullptr };
        std::vector<glm::vec4> _placeholderTextColors { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };

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
        bool hasInputFocus() const;
    };
}