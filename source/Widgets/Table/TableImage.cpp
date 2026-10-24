#include "TableImage.hpp"
#include "ITexture.hpp"
#include <algorithm>

RetroFuturaGUI::TableImage::TableImage(Table* parentTable, Projection* projection, Rectangle* rectangle)
    : ITableWidget(parentTable, projection), _rectangle(rectangle)
{
    _tableWidgetID = TableWidgetTypeID::TableImage;
}

void RetroFuturaGUI::TableImage::SetTexture(const ITexture* texture)
{
    _texture = texture;
}

void RetroFuturaGUI::TableImage::SetPadding(const f32 padding)
{
    _padding = std::max(0.0f, padding);
}

void RetroFuturaGUI::TableImage::Draw()
{
    if(!_rectangle)
        return;

    if(!_texture)
        return;

    const f32 aspectRatio { _texture->GetAspectRatio() };

    if(0.0f >= aspectRatio)
        return;

    //As wide as the cell allows, then as tall as that width makes it
    const glm::vec2 room { std::max(0.0f, _size.x - 2.0f * _padding), std::max(0.0f, _size.y - 2.0f * _padding) };
    const f32 width { std::min(room.x, room.y * aspectRatio) };

    if(0.0f >= width)
        return;

    _rectangle->SetBackgroundImageTextureID(_texture->GetID());
    _rectangle->SetSize(glm::vec2(width, width / aspectRatio));
    _rectangle->SetPosition(_position);
    _rectangle->SetRotation(_rotation);
    _rectangle->Draw();
}