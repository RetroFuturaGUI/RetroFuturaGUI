#pragma once
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextEditable.hpp"
#include <memory>
#include <span>

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
        void Draw() override;
        void SetEnabled(const bool enable, const bool emitSignal = true);
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetCornerRadii(const glm::vec4& radii);
        void SetSelectedAreaColors(std::span<glm::vec4> colors);
        void SetSelectedAreaFillType(const FillType fillType);
        void SetSelectedAreaGradientAnimationSpeed(const f32 speed);
        void SetSelectedAreaGradientOffset(const f32 gradientOffset);
        void SetSelectedAreaGradientDegree(const f32 degree);
        void SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed);
        void SetSelectedAreaCornerRadii(const glm::vec4& radii);

    //Geometry
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;

    private:
        void interact();
        void setColors(const ColorState state);
        void drawCaret();
        void drawSelectedArea();
        void updateSelectedArea();
        void setCaretFromBoundary(const uSize boundary);
        f32 clampToTextBounds(const f32 worldX, const f32 halfExtent = 0.0f) const override;
        f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f) override;

        std::unique_ptr<Rectangle> _selectedArea;
        std::vector<glm::vec4> _selectedAreaColors { glm::vec4(0.24f, 0.47f, 0.85f, 0.4f) };
        uSize
            _markedPositionFirst { 0 },
            _markedPositionLast { 0 };
        f32 _scrollOffsetX { 0.0f };
        bool
            _isMarking { false },
            _isSelected { false };
    };
}