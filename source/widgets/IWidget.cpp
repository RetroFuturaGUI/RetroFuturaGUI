#include "IWidget.hpp"

IWidget::IWidget(const std::string& name, i32 width, i32 height, void* parent, Sizing minWidth, Sizing minHeight, Sizing maxWidth, Sizing maxHeight) 
        : IWidgetBase(name, parent)
{
    _width = width;
    _height = height;

    if(minWidth == Sizing::UseCurrent)
        _minWidth = width;
    else
        _minWidth = static_cast<i32>(minWidth);

    if(minHeight == Sizing::UseCurrent)
        _minHeght = height;
    else
        _minHeght = static_cast<i32>(minHeight);

    if(maxWidth == Sizing::UseCurrent)
        _maxWidth = width;
    else
        _maxWidth = static_cast<i32>(maxWidth);

    if(maxHeight == Sizing::UseCurrent)
        _maxHight = height;
    else
        _maxHight = static_cast<i32>(maxHeight);
}

i32 IWidget::GetMinWidth() const
{
    return _minWidth;
}

void IWidget::SetMinWidth(const i32 minWidth)
{
    _minWidth = minWidth;
}

i32 IWidget::GetMinHeght() const
{
    return _minHeght;
}

void IWidget::SetMinHeght(const i32 minHeight)
{
    _minHeght = minHeight;
}

i32 IWidget::GetMaxWidth() const
{
    return _maxWidth;
}

void IWidget::SetMaxWidth(const i32 maxWidth)
{
    _maxWidth = maxWidth;
}

i32 IWidget::GetMaxHight() const
{
    return _maxHight;
}

void IWidget::SetMaxHight(const i32 maxHeight)
{
    _maxHight = maxHeight;
}

i32 IWidget::GetWidth() const
{
    return _width;
}

void IWidget::SetWidth(const i32 width)
{
    _width = width;
}

i32 IWidget::GetHeight() const
{
    return _height;
}

void IWidget::SetHeight(const i32 height)
{
    _height = height;
}

glm::vec4 IWidget::GetBackgroundColor() const
{
    return _backgroundColor;
}

void IWidget::SetBackgroundColor(const glm::vec4& backgroundColor)
{
    _backgroundColor = backgroundColor;
}