#pragma once
#include "ITextProperties.hpp"
#include "RadioButton.hpp"
#include "Text.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    class RadioButton;

    class RadioButtonGroup : public IWidget, public IBorder, public ITextProperties
    {
    public:
        /// @brief Constructs a RadioButtonGroup under the given parent widget/window.
        RadioButtonGroup(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        RadioButtonGroup(const RadioButtonGroup&) = delete;
        RadioButtonGroup(RadioButton&&) = delete;
        ~RadioButtonGroup() = default;
        auto operator =(const RadioButtonGroup&) = delete;
        auto operator =(RadioButtonGroup&&) = delete;

        void Draw() override;

        /// @brief Sets the size of the radio button group.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the radio button group.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the radio button group.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief deselcts all registered RadioButtons except the passed one
        /// @param a pointer of the RadioButton to stay checked
        void DeselectOthers(RadioButton* activeRadioButton);

        /// @brief registers a new RadioButton
        /// @param a pointer to the new RadioButton
        void RegisterRadioButton(RadioButton* newRadioButton);

        /// @brief unregisters a RadioButton
        /// @param a pointer to the RadioButton to be unregistered
        void UnregisterRadioButton(RadioButton* obsoleteRadioButton);

    private:
        std::vector<RadioButton*> _radioButtonRefs {};

    };
}