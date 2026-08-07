
#pragma once
#include "ITextProperties.hpp"

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
        void Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot);
        void Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot);

    protected:
        void moveCaret();
        void editText();

        //Lets widgets with fixed geometry (e.g. TextBox) keep the caret from rendering past their own edges.
        //Default is a no-op so ITextEditable stays usable by widgets without such bounds. Pure/side-effect-free -
        //used for one-off clipping (e.g. the selection highlight), where the view itself must NOT be scrolled.
        virtual f32 clampToTextBounds(const f32 worldX, const f32 = 0.0f) const { return worldX; }

        //Called only when the caret itself moves. Unlike clampToTextBounds, this may scroll the text into view
        //(mutating state) so the caret stays visible instead of just being pinned to the edge. Default falls
        //back to the pure clamp above, for widgets without scrolling support.
        virtual f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f) { return clampToTextBounds(worldX, halfExtent); }

        std::unique_ptr<Rectangle> _caret;
        uSize _caretPosition { 0 };
        CaretRelativePosition _caretRelativePosition { CaretRelativePosition::Right };
        std::vector<glm::vec4> _caretColors { glm::vec4(1.0f) };
        bool
            _textChangedFlag { false },
            _readOnly { false },
            _editingEnabled { false },
            _keyWasReleased { true },
            _enterPressed { false },
            _showCaret { false };
        u32 _keyHoldFrames { 0 };
        std::u32string _keyRepeatText {};
        std::vector<char> _prevKeyStates {};
        static constexpr i32 _keyRepeatInitialDelay = 60;
        static constexpr i32 _keyRepeatInterval = 5;

        Signal<>
            _onEnterPressed,
            _onEnterPressedAsync,
            _onEnterReleased,
            _onEnterReleasedAsync;

    private:
        void moveCaretImpl();
        void emitEnterRelease();
        void emitEnterPressed();
        void emitChange();
        void updateCaretPosition();
    };
}