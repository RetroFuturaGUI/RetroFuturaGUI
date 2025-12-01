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
        

    private:
        std::unique_ptr<Text> _textObject;

    };
}