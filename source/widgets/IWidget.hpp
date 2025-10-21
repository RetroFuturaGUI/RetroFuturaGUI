#pragma once
#include "IWidgetBase.hpp"
#include <glm/glm.hpp>
//#include "FVec.hpp"

class IWidget : public IWidgetBase
{
public:
    IWidget(const std::string& name, i32 width, i32 height, void* parent = nullptr, Sizing minWidth = Sizing::UseCurrent, Sizing minHeight = Sizing::UseCurrent, Sizing maxWidth = Sizing::UseCurrent, Sizing maxHeight = Sizing::UseCurrent);

    i32 GetMinWidth() const;
    void SetMinWidth(const i32 minWidth);
    i32 GetMinHeght() const;
    void SetMinHeght(const i32 minHeight);
    i32 GetMaxWidth() const;
    void SetMaxWidth(const i32 maxWidth);
    i32 GetMaxHight() const;
    void SetMaxHight(const i32 maxHeight);
    i32 GetWidth() const;
    void SetWidth(const i32 width);
    i32 GetHeight() const;
    void SetHeight(const i32 height);
    glm::vec4 GetBackgroundColor() const;
    void SetBackgroundColor(const glm::vec4& backgroundColor);

protected:
    i32 _minWidth = 0;
    i32 _minHeght = 0;
    i32 _maxWidth = 0;
    i32 _maxHight = 0;
    i32 _width = 0;
    i32 _height = 0;
    glm::vec4 _backgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec4 _borderWidth = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 _borderColor = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 _borderCornerRadius = {0.0f, 0.0f, 0.0f, 0.0f};

private:

};