#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <string>
#include <vector>
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    struct LasagnaCell
    {
        glm::vec3
            _PositionPixels { 0.0f },
            _PositionNormalized { 0.0f },
            _SizePixels { 0.0f },
            _SizeNormalized { 0.0f },
            _PaddingPixels { 0.0f },
            _PaddingNormalized { 0.0f };
        u32 
            _RowSpan { 1 },
            _ColSpan { 1 },
            _LayerSpan { 1 };
        IWidget* _Widget = nullptr;
        bool _SpanOccupied = false;
        SizingMode _SizingMode { SizingMode::FILL };
    };

    struct AxisDefinition
    {
        std::vector<f32>
            _RowDefinition {},
            _ColumnDefinition {},
            _LayerDefinition {};
    };

    class Lasagna: public IWidget
    {
    public:
        Lasagna(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, AxisDefinition* _axisDefinition);
        Lasagna() = delete;
        Lasagna(const Lasagna&) = delete;
        Lasagna(Lasagna&&) = delete;
        ~Lasagna() = default;
        auto operator =(const Lasagna&) = delete;
        auto operator =(Lasagna&&) = delete;
        void AttachWidget(const u32 row, const u32 col, const u32 layer, IWidget* widget, const SizingMode sizingMode = SizingMode::FILL);
        void Draw() override {};
        void Draw(const bool alsoDrawDebugLines = false);
        void SetSize(const glm::vec3& size);
        void SetPosition(const glm::vec3& position);

    private:
        // Data
        std::vector<std::vector<std::vector<LasagnaCell>>> _lasagna;
        static constinit const u32 _maxCountPerAxis = 64;
        bool _drawDebugLines = false;
        AxisDefinition _axisdefinition;
        std::unique_ptr<Rectangle> _debugBorder;
        std::vector<glm::vec4> _debugBorderColor { glm::vec4(1.0f) };

        void drawDebugLines(const LasagnaCell& cell);
        void resizeCells();
        void resizeWidget(const LasagnaCell& cell);
        void resizeAllWidgets();
        void moveWidgets();
    };
}