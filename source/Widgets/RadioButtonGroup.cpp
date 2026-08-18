#include "RadioButtonGroup.hpp"
#include "RadioButton.hpp"
#include <algorithm>

void RetroFuturaGUI::RadioButtonGroup::DeselectOthers(RadioButton* activeRadioButton)
{
    for(RadioButton* ref : _radioButtonRefs)
        if(ref != activeRadioButton)
            ref->SetValue(false, false);
}

void RetroFuturaGUI::RadioButtonGroup::RegisterRadioButton(RadioButton* newRadioButton)
{
    _radioButtonRefs.push_back(newRadioButton);
}

void RetroFuturaGUI::RadioButtonGroup::UnregisterRadioButton(RadioButton* obsoleteRadioButton)
{
    _radioButtonRefs.erase(
        std::remove_if(_radioButtonRefs.begin(), _radioButtonRefs.end(), 
        [&](RadioButton* ptr) { return obsoleteRadioButton == ptr; })
    );
}