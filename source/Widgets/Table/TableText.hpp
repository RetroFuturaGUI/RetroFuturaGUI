#pragma once
#include "ITableWidget.hpp"
#include "ITableTextProperties.hpp"
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    class TableText : public ITableWidget, public ITableTextProperties
    {
        public:
            TableText(Table* parentTable, Projection* projection);
            TableText() = delete;

            void Draw() override;
            void SetSize(const glm::vec3& size) override;
            void SetPosition(const glm::vec3& position) override;
            void SetRotation(const glm::vec3& rotation) override;
            void SetText(std::string_view text) override;
            void SetTextColors(std::span<glm::vec4> colors, const ColorState colorState);


        private:
  
    };
}