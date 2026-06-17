
#pragma once
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with editable text capabilities.
    class ITextEditable : virtual public IWindowAccessor, public ITextProperties
    {
    public:

    protected:
        void moveCaret();
        
        std::unique_ptr<Rectangle> _caret;
        bool _showCaret { false };
        uSize _caretPosition { 0 };
        CaretRelativePosition _caretRelativePosition { CaretRelativePosition::Right };
        std::vector<glm::vec4> _caretColors { glm::vec4(1.0f) };

    private:
        void moveCaretImpl();


    };
}