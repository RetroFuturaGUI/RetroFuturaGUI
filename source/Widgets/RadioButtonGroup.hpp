#pragma once
#include "IWidget.hpp"
#include "RadioButton.hpp"

namespace RetroFuturaGUI
{
    class RadioButtonGroup : public IWidget
    {
    public:

        void DeselectOthers(RadioButton* activeRadioButton);
        void RegisterRadioButton(RadioButton* newRadioButton);
        void UnregisterRadioButton(RadioButton* obsoleteRadioButton);
    };
}