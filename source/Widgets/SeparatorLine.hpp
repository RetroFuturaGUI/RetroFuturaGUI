#pragma once
#include "IBackground.hpp"
#include "ITextProperties.hpp"
#include "IWidget.hpp"
#include "config.hpp"
#include <string_view>

namespace RetroFuturaGUI
{
    class SeparatorLine final : public IWidget, public ITextProperties, public IBackground
    {
    public:
        SeparatorLine(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        SeparatorLine() = delete;
        SeparatorLine(const SeparatorLine&) = delete;
        SeparatorLine(SeparatorLine&&) = delete;
        auto operator =(const SeparatorLine&) = delete;
        auto operator =(SeparatorLine&&) = delete;

        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Sets the text content, in UTF-8, and resizes the gap holding it to match.
        void SetText(std::string_view text, const bool emitSignal) override;

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;

        /// @brief Sets the padding applied around the text, which widens the gap on both sides.
        void SetTextPadding(const f32 padding) override;

        /// @brief Sets how far from the line's left edge the text - and so its gap - starts.
        void SetTextLeftPadding(const f32 padding);

        /// @brief Sets whether the text overlay is drawn. While it is, the line carries a gap sized
        /// to hold it, so the background gap is driven from here rather than set directly.
        void ShowText(const bool enable);

    private:
        /// @brief Sizes and places the gap the text sits in, and centers the text inside it.
        void updateLayout();

        bool _showText { false };
        f32 
            _textLeftPadding { 50.0f };
    };
}