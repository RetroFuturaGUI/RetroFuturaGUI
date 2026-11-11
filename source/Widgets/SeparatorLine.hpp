#pragma once
#include "Background.hpp"
#include "WidgetText.hpp"
#include "IWidget.hpp"
#include "config.hpp"
#include <string_view>

namespace RetroFuturaGUI
{
    class SeparatorLine final : public IWidget
    {
    public:
        SeparatorLine(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        SeparatorLine() = delete;
        SeparatorLine(const SeparatorLine&) = delete;
        SeparatorLine(SeparatorLine&&) = delete;
        auto operator =(const SeparatorLine&) = delete;
        auto operator =(SeparatorLine&&) = delete;

        void Draw() override;

        /// @brief Returns the widget's text: its string, font, colors and effects. The gap in the line holds the text and its
        ///        padding, and the text's alignment places it. Left the edge padding from the line's left end, Right as far from
        ///        its right end, Center in the middle. A caption longer than the line fills it and is cut at its ends, showing its
        ///        start, middle or end as aligned
        WidgetText& GetText();
        const WidgetText& GetText() const;

        /// @brief Sets how far the gap stays from the end of the line the text is aligned to (50 by default). Unused while centered,
        ///        and never more than the line leaves beside the gap
        void SetTextEdgePadding(const f32 padding);

        /// @brief Sets whether the text overlay is drawn. While it is, the line carries a gap sized
        /// to hold it, so the background gap is driven from here rather than set directly
        void ShowText(const bool enable);

        /// @brief Returns the line's background
        Background& GetBackground();
        
        /// @brief Returns the line's background
        const Background& GetBackground() const;


    protected:
        void applySize(const glm::vec3& size) override;

        void applyPosition(const glm::vec3& position) override;

    private:
        /// @brief Sizes and places the gap the text sits in, as the text is aligned, and makes the gap the text's box
        void updateLayout();

        Background _background { &_projection };
        WidgetText _text { &_projection, [this](const WidgetTextChange) { updateLayout(); } }; //the text's size is the gap's
        f32 _textEdgePadding { 50.0f };
        bool _showText { false };
    };
}