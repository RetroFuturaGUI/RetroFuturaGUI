#pragma once
#include "RadioButton.hpp"

namespace RetroFuturaGUI
{
    class RadioButton;
    
    class RadioButtonGroup : public IWidget
    {
    public:

        void DeselectOthers(RadioButton* activeRadioButton);
        void RegisterRadioButton(RadioButton* newRadioButton);
        void UnregisterRadioButton(RadioButton* obsoleteRadioButton);

    private:
        std::vector<RadioButton*> _radioButtonRefs {};
    };
}