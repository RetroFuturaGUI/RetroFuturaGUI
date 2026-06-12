#pragma once
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    //A text-based widget that shows information as text
    class Label : public IWidget, public ITextProperties
    {
    public:
        Label(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Label() = delete;
        Label(const Label&) = delete;
        Label(Label&&) = delete;
        auto operator =(const Label&) = delete;
        auto operator =(Label&&) = delete;
        ~Label() = default;
        void Draw() override;
        void SetEnabled(const bool enable);
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const float rotation) override;

    private:
        void setColors(const ColorState state);
    };
}