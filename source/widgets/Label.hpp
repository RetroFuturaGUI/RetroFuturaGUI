#pragma once
#include "Text.hpp"
#include "IWidget.hpp"

namespace RetroFuturaGUI
{
    class Label : public IWidget
    {
    public:

        Label(const IdentityParams& identity, const GeometryParams2D& geometry, const TextParams& textParams);
        ~Label() = default;
        void Draw() override;
        void SetEnabled(const bool enable);
        void SetTextColors(std::span<glm::vec4> colors, const ColorState colorState);

    private:
        std::unique_ptr<Text> _textObject;
        // Style
        std::vector<glm::vec4> 
            _textColorEnabled { glm::vec4(1.0f) },
            _textColorDisabled { glm::vec4(0.5f, 0.5f,0.5f, 1.0f) },
            //_textColorClicked { glm::vec4(1.0f) }, //might be used for link features
            _textColorHover { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };

        void setColors();
    };
}