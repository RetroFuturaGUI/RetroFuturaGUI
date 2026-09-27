#pragma once
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include <memory>
#include <vector>
#include "IBorder.hpp"
#include "config.hpp"

namespace RetroFuturaGUI
{
    class ColorPreview final : public IWidget, public IBorder
    {
    public:
        enum class DualPrevieAlignment : u32
        {
            Horizontal,
            Vertical
        };

        ColorPreview() = delete;
        ColorPreview(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~ColorPreview() = default;
        ColorPreview(const ColorPreview&) = delete;
        ColorPreview(ColorPreview&&) = delete;
        auto operator =(const ColorPreview&) = delete;
        auto operator =(ColorPreview&&) = delete;
        
        /// @brief Returns the color currently being previewed.
        const glm::vec4& GetColor() const;

        /// @brief Sets the color to preview. Its alpha lets the checkerboard behind it show through.
        void SetPreviewColor(const glm::vec4& color);

        void Draw() override;

        /// @brief Sets the size of the whole widget. While the dual preview is on, the two backgrounds split it in half.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position, stacking the previewed color and the border in front of the backgrounds.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the backgrounds, the previewed color and the border alike.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Enables the dual preview: the checkerboard gives up half the widget to a white background,
        /// so the same color reads against transparency on one half and against white on the other.
        void EnableDualPreviewBackground(const bool enable);

        /// @brief Sets which way the dual preview is split: Horizontal puts the halves side by side, Vertical stacks them.
        void SetDualPreviewAlignment(const DualPrevieAlignment alignment);

    private:
        /// @brief Sizes and places both backgrounds, the previewed color and the border for the current split.
        void alignColorPreview();

        inline static constexpr const glm::vec4
            _primaryCheckeredColor { 1.0f, 1.0f, 1.0f, 1.0f },
            _secondaryCheckeredColor { 0.8f, 0.8f, 0.8f, 1.0f };
        glm::vec4
            _previewColor { 1.0f };
        std::unique_ptr<Rectangle>
            _checkeredBackground { nullptr },
            _whiteBackground { nullptr },
            _colorRect { nullptr };
        DualPrevieAlignment _previewAlignment { DualPrevieAlignment::Horizontal };
        bool _useWhiteBackgroundRect { true };

        /// @brief The fill the checkerboard's squares are drawn over. A Rectangle draws nothing at all until it holds a color.
        std::vector<glm::vec4> _checkeredFillColor { _primaryCheckeredColor };

        /// @brief The white half's fill, kept here because a Rectangle holds a span over the colors it is given.
        std::vector<glm::vec4> _whiteBackgroundColor { glm::vec4(1.0f) };

        /// @brief Side length of every checkerboard square, in pixels. A Rectangle keeps a span over this, so it has to outlive the call.
        std::vector<f32> _checkeredSquareWidth { 8.0f };
    };
}