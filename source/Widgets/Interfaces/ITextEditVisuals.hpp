#pragma once
#include "ITextInteraction.hpp"
#include "Rectangle.hpp"
#include <memory>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    //An interface drawing the caret and selection of all editable text types
    class ITextEditVisuals : public ITextInteraction
    {
    public:
        /// @brief Sets the caret color(s).
        void SetCaretColors(std::span<glm::vec4> colors);

        /// @brief Sets the caret fill type (solid, linear/radial/huestar gradient).
        void SetCaretFillType(const FillType fillType);

        /// @brief Sets the speed at which the caret's gradient animates over time.
        void SetCaretGradientAnimationSpeed(const f32 speed);

        /// @brief Sets how long, in milliseconds, the caret stays visible/hidden per blink cycle.
        void SetCaretBlinkTime(const f64 milliseconds);

        /// @brief Sets the selection highlight color(s).
        void SetSelectedAreaColors(std::span<glm::vec4> colors);

        /// @brief Sets the selection highlight fill type (solid, linear/radial/huestar gradient).
        void SetSelectedAreaFillType(const FillType fillType);

        /// @brief Sets the speed at which the selection highlight's gradient animates over time.
        void SetSelectedAreaGradientAnimationSpeed(const f32 speed);

        /// @brief Sets the offset applied to the selection highlight gradient's start position.
        void SetSelectedAreaGradientOffset(const f32 gradientOffset);

        /// @brief Sets the angle of the selection highlight's linear gradient, in degrees.
        void SetSelectedAreaGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the selection highlight's gradient rotates over time.
        void SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the corner rounding radii of the selection highlight.
        void SetSelectedAreaCornerRadii(const glm::vec4& radii);

    protected:
        struct TextArea
        {
            f32
                _Left { 0.0f },
                _Bottom { 0.0f },
                _Right { 0.0f },
                _Top { 0.0f };
        };

        /// @brief The world-space area the edited text is visible in. The caret and selection are clipped to it.
        virtual TextArea textArea() const = 0;

        /// @brief Creates the caret and selection rectangles. Owners call it from their constructor.
        void initEditVisuals(Projection* projection);

    //ITextInteraction hooks
        void updateCaretPosition() override final;
        void updateSelectedArea() override final;

        void drawSelectedArea();
        void drawCaret();

        f32 clampToTextBounds(const f32 worldX, const f32 halfExtent = 0.0f) const;
        f32 clampToTextBoundsY(const f32 worldY) const;

        /// @brief Scrolls the edited text sideways so the caret stays inside the text area, and returns the caret's x after scrolling.
        f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f);

        /// @brief Returns the caret's full height for the edited text, before updateCaretPosition clips it to the text area.
        f32 caretHeight() const;

    //Caret
        std::unique_ptr<Rectangle> _caret {};
        std::vector<glm::vec4> _caretColors { glm::vec4(1.0f) };
        static constexpr f32 _caretHeightFactor { 1.6f };
        bool _isCaretInView { true };

    //Selection
        struct SelectedLineArea
        {
            glm::vec2
                _Center { 0.0f },
                _Size { 0.0f };
        };
        std::unique_ptr<Rectangle> _selectedArea {};
        std::vector<SelectedLineArea> _selectedLineAreas {};
        std::vector<glm::vec4> _selectedAreaColors { glm::vec4(0.24f, 0.47f, 0.85f, 0.4f) };
    };
}
