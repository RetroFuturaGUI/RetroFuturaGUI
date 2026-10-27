
#pragma once
#include "ITextProperties.hpp"
#include "ITextEditVisuals.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with editable text capabilities.
    class ITextEditable : virtual public IWindowAccessor, public ITextProperties, public ITextEditVisuals
    {
    public:
        /// @brief Sets whether the widget rejects text input/editing while still allowing selection and copy.
        void EnableReadOnly(const bool enable);

        /// @brief Returns whether the widget is currently read-only.
        bool IsReadOnly() const;

        /// @brief Sets the placeholder text colors.
        void SetPlaceholderTextColors(std::span<const glm::vec4> colors);

        /// @brief Sets the placeholder text content, in UTF-8.
        void SetPlaceholderText(std::string_view text);

        /// @brief Returns the placeholder text content, in UTF-8.
        const std::string& GetPlaceholderText() const;

        /// @brief Sets the font family, size and style used to render the text and placeholder text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) final override;

        /// @brief Sets the horizontal alignment of the text and placeholder text.
        void SetTextAlignment(const TextAlignment alignment) final override;

        /// @brief Sets the padding applied around the text and placeholder text.
        void SetTextPadding(const f32 padding) final override;

    protected:
    //ITextInteraction hooks: this widget edits the single Text it owns. ITextEditVisuals draws its caret and selection.
        Text* getActiveText() const override final;
        void emitChange() override;
        bool isTextReadOnly() const override final;

    //input logic
        bool _readOnly { false };
        std::vector<char> _prevKeyStates {};

    //Placeholder Text
        std::unique_ptr<Text> _placeholderText { nullptr };
        std::vector<glm::vec4> _placeholderTextColors { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };
    };
}