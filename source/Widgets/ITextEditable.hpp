
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

    protected:
        void moveCaret();
        void editText();
        
        std::unique_ptr<Rectangle> _caret;
        bool _showCaret { false };
        uSize _caretPosition { 0 };
        CaretRelativePosition _caretRelativePosition { CaretRelativePosition::Right };
        std::vector<glm::vec4> _caretColors { glm::vec4(1.0f) };
        bool
            _textChangedFlag { false },
            _readOnly { false },
            _editingEnabled { false },
            _keyWasReleased { true };
        u32 _keyHoldFrames { 0 };
        std::u32string _keyRepeatText {};
        std::vector<char> _prevKeyStates {};
        static constexpr i32 _keyRepeatInitialDelay = 60;
        static constexpr i32 _keyRepeatInterval = 5;

    private:
        void moveCaretImpl();


    };
}