#pragma once
#include "ITextInteraction.hpp"
#include "Caret.hpp"
#include "SelectedArea.hpp"
#include <memory>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    //An interface drawing the caret and selection of all editable text types
    class ITextEditVisuals : public ITextInteraction
    {
    public:
        /// @brief Returns the caret for styling: GetCaret().SetColors(...)
        Caret& GetCaret();
        const Caret& GetCaret() const;

        /// @brief Returns the highlight behind the selection for styling: GetSelectedArea().SetColors(...)
        SelectedArea& GetSelectedArea();
        const SelectedArea& GetSelectedArea() const;

    protected:
        struct TextArea
        {
            f32
                _Left { 0.0f },
                _Bottom { 0.0f },
                _Right { 0.0f },
                _Top { 0.0f };
        };

        /// @brief The world-space area the edited text is visible in. The caret and selection are clipped to it
        virtual TextArea getTextArea() const = 0;

        /// @brief Creates the caret and the selection's highlight
        explicit ITextEditVisuals(Projection* projection);

        /// @brief Flips the caret's visibility once a full blink interval has elapsed
        void updateCaretBlink();

    //ITextInteraction hooks
        void updateCaretPosition() override final;
        void updateSelectedArea() override final;

        void drawSelectedArea();
        void drawCaret();

        f32 clampToTextBounds(const f32 worldX, const f32 halfExtent = 0.0f) const;
        f32 clampToTextBoundsY(const f32 worldY) const;

        /// @brief Scrolls the edited text sideways so the caret stays inside the text area, and returns the caret's x after scrolling
        f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f);

        /// @brief Returns the caret's full height for the edited text, before updateCaretPosition clips it to the text area
        f32 getCaretHeight() const;

    //Caret
        Caret _caret;
        static constexpr f32 _caretHeightFactor { 1.6f };
        static constexpr f32 _caretThickness { 2.0f };
        bool _isCaretInView { true };

    //Selection
        SelectedArea _selectedArea;
        std::vector<SelectedArea::LineArea> _selectedLineAreas {};
    };
}