#include "Label.hpp"

RetroFuturaGUI::Label::Label(const IdentityParams& identity, const GeometryParams2D& geometry, const TextParams& textParams)
: IWidget(identity, geometry)
{
    _textObject = std::make_unique<Text>(geometry, textParams);



}

void RetroFuturaGUI::Label::Draw()
{
    _textObject->Draw();
}
