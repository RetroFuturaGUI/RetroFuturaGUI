#pragma once
#include "ITableWidget.hpp"
#include "ITableTextProperties.hpp"
#include "ITypedText.hpp"
#include "IncludeHelper.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    class TableText final : public ITableWidget, public ITableTextProperties, public ITypedText
    {
        public:
            TableText(Table* parentTable, Projection* projection);
            TableText() = delete;

            void Draw() override;
            void SetSize(const glm::vec3& size) override;
            void SetPosition(const glm::vec3& position) override;
            void SetRotation(const glm::vec3& rotation) override;
            /// @brief Sets the cell's text verbatim. Does not create a value store; updates one only if it exists.
            void SetText(std::string_view text) override;
            void SetTextColors(std::span<glm::vec4> colors, const ColorState colorState);

            /// @brief Stores the edited cell text as its value. Table calls it whenever an edit changes the text.
            void syncValueFromText();

        private:
            void renderValueText(std::string_view text, const bool) override; //Table emits the cell's signals itself
    };
}
