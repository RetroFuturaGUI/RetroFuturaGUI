#pragma once
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextEditable.hpp"

namespace RetroFuturaGUI
{
    //A single-line text input widget
    class TextBox : public IWidget, public IClickable, public IBackground, public IBorder, public ITextEditable
    {
    public:
        TextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        TextBox() = delete;
        TextBox(const TextBox&) = delete;
        TextBox(TextBox&&) = delete;
        ~TextBox() = default;
        auto operator =(const TextBox&) = delete;
        auto operator =(TextBox&&) = delete;
        void Draw();
        void Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnTextChange(const typename Signal<>::Slot& slot);
        void SetEnabled(const bool enable, const bool emitSignal = true);
        void SetCaretColors(std::span<glm::vec4> colors);
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetCornerRadii(const glm::vec4& radii);

    //Geometry
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const float rotation) override;

    private:
        void interact();
        void setColors(const ColorState state);
        void drawCaret();
    };
}