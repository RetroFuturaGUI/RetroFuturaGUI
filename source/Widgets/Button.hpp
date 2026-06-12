#pragma once
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IDesignable.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    //A widget that triggers actions
    class Button : public IWidget, public IClickable, public IDesignable, public ITextProperties
    {
    public:
        Button(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Button() = delete;
        Button(const Button&) = delete;
        Button(Button&&) = delete;
        auto operator =(const Button&) = delete;
        auto operator =(Button&&) = delete;
        ~Button() = default;
        void Draw();
        void SetEnabled(const bool enable, const bool emitSignal = true);
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const float rotation) override;

    private:
        void interact();
        void setColors(const ColorState state);
    };
}