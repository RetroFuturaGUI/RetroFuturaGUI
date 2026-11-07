
#pragma once
#include "WidgetText.hpp"
#include "ITextEditVisuals.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with editable text capabilities
    class ITextEditable : virtual public IWindowAccessor, public ITextEditVisuals
    {
    public:
        /// @brief Sets whether the widget rejects text input/editing while still allowing selection and copy
        void EnableReadOnly(const bool enable);

        /// @brief Returns whether the widget is currently read-only
        bool IsReadOnly() const;

        /// @brief Returns the edited text's string, font, colors and effects. Its font, alignment and padding also apply to the placeholder
        WidgetText& GetText();
        const WidgetText& GetText() const;

        /// @brief Returns the text shown while the edited text is empty
        WidgetText& GetPlaceholder();
        const WidgetText& GetPlaceholder() const;

    protected:
        /// @brief Creates the text and the placeholder
        explicit ITextEditable(Projection* projection);
    //ITextInteraction hooks: this widget edits the single Text it owns. ITextEditVisuals draws its caret and selection
        Text* getActiveText() const override final;
        void emitChange() override;
        bool isTextReadOnly() const override final;

        /// @brief Called when the text's string, font or padding change, before OnTextChange fires
        virtual void textChanged(const WidgetTextChange) {}

    //input logic
        bool _readOnly { false };
        std::vector<char> _prevKeyStates {};

    //Text
        WidgetText _placeholder; //follows _text, so it is declared before it
        mutable WidgetText _text; //getActiveText hands it out for editing, also from const functions
    };
}