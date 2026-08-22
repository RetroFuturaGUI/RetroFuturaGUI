#include "RadioButtonGroup.hpp"
#include "ITextProperties.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "RadioButton.hpp"
#include "Rectangle.hpp"
#include "config.hpp"
#include <algorithm>
#include <cstddef>
#include <glm/ext/vector_int2_sized.hpp>
#include <memory>

RetroFuturaGUI::RadioButtonGroup::RadioButtonGroup(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::RadioButtonGroup;
    _border = std::make_unique<Rectangle>(projection);
    _text = std::make_unique<Text>(projection);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::RadioButtonGroup::Draw()
{
    drawBorder();

    if(_text)
        _text->Draw();

    for(const auto& row : _grid)
        for(const auto& column: row)
        {
            if(!column._RadioButton)
                continue;

            column._RadioButton->Draw();

            if(column._Label)
                column._Label->Draw();
        }
}

void RetroFuturaGUI::RadioButtonGroup::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    updateLabelLayout();
    updateGridLayout();
}

void RetroFuturaGUI::RadioButtonGroup::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    updateLabelLayout();
    updateGridLayout();
}

void RetroFuturaGUI::RadioButtonGroup::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);
}

void RetroFuturaGUI::RadioButtonGroup::DeselectOthers(RadioButton* activeRadioButton)
{
    for(auto& row : _grid)
        for(auto& column : row)
        {
            if(!column._RadioButton)
                continue;
            
            if(column._RadioButton != activeRadioButton)
                column._RadioButton->SetValue(false, false);
        }
}

void RetroFuturaGUI::RadioButtonGroup::RegisterRadioButton(RadioButton* newRadioButton, Label* label, const glm::i64vec2& index)
{
    _grid[index.x][index.y]._RadioButton = newRadioButton;
    _grid[index.x][index.y]._Label = label;
    updateGridLayout();
}

void RetroFuturaGUI::RadioButtonGroup::UnregisterRadioButton(RadioButton* obsoleteRadioButton)
{
    for(auto& row : _grid)
        for(auto& column : row)
            if(column._RadioButton == obsoleteRadioButton)
            {
                column._RadioButton = nullptr;
                column._Label = nullptr;
                return;
            }
}

void RetroFuturaGUI::RadioButtonGroup::SetText(std::string_view text, const bool emitSignal)
{
    ITextProperties::SetText(text, emitSignal);
    updateLabelLayout();
}

void RetroFuturaGUI::RadioButtonGroup::updateLabelLayout()
{
    if(!_text || !_border)
        return;

    if(GetText().empty()) //if no label, draw solid border
    {
        _border->SetSize(glm::vec2(_size.x, _size.y));
        _border->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
        _text->SetParentSize(glm::vec2(_size.x, _size.y));
        IBorder::SetBorderGaps(std::span<BorderGap>());
        return;
    }

    f32 
        glyphHeight { _text->GetHeight() },
        textPadding { _text->GetPadding() },
        textWidth { _text->GetWidth() };

    glm::vec2 newSize { _size.x, _size.y - glyphHeight * 0.5f };
    glm::vec3 newPos { _position.x, _position.y + glyphHeight * 0.5f, _position.z + 0.01f };
    _border->SetSize(newSize);
    _border->SetPosition(newPos);
    _text->SetParentSize(newSize);

    // single top border's edge gap, sized to fit the label (+ padding on both sides)
    BorderGap textGap {
        .edge = BorderEdge::Top,
        .offset = _textLeftPadding,
        .length = textWidth + textPadding * 2.0f,
        .anchorFarCorner = false,
        .repeat = 1
    };

    IBorder::SetBorderGaps(std::span<BorderGap>(&textGap, 1));
    _text->SetTextAlignment(TextAlignment::Center);
    _text->SetPosition(glm::vec3(
        newPos.x - newSize.x * 0.5f + _textLeftPadding + textGap.length * 0.5f,
        newPos.y + newSize.y * 0.5f,
        newPos.z + 0.01f)
    );
}

void RetroFuturaGUI::RadioButtonGroup::SetBorderGaps(std::span<BorderGap> gaps)
{
    IBorder::SetBorderGaps(gaps);

    for(const BorderGap& gap : gaps)
        if(gap.edge == BorderEdge::Left)
        {
            _textLeftPadding = gap.offset;
            return;
        }
}

void RetroFuturaGUI::RadioButtonGroup::SetAxisDefinitions(std::span<f32> rowDefinitions, std::span<f32> columnDefinitions)
{
    _rowDefinitions.assign(rowDefinitions.begin(), rowDefinitions.end());
    _columnDefinitions.assign(columnDefinitions.begin(), columnDefinitions.end());
    _grid.assign(_rowDefinitions.size(), std::vector<GridCell>(_columnDefinitions.size()));
    updateGridLayout();
}

void RetroFuturaGUI::RadioButtonGroup::SetGridContentAlignment(const TextAlignment alignment)
{
    _gridContentAlignment = alignment;
    updateGridLayout();
}

void RetroFuturaGUI::RadioButtonGroup::updateGridLayout()
{
    const f32 leftX { _position.x - _size.x * 0.5f };
    f32 cursorY { _position.y + _size.y * 0.5f };

    for(uSize row { 0 }; row < _rowDefinitions.size(); ++row)
    {
        const f32 rowHeight { _rowDefinitions[row] * _size.y };
        f32 cursorX { leftX };

        for(uSize column { 0 }; column < _columnDefinitions.size(); ++column)
        {
            const f32 columnWidth { _columnDefinitions[column] * _size.x };
            GridCell& cell { _grid[row][column] };
            cell._Size = glm::vec2(columnWidth, rowHeight);
            cell._Position = glm::vec2(cursorX + columnWidth * 0.5f, cursorY - rowHeight * 0.5f);

            // RadioButton + label sit side by side separated by the label's text padding)
            const f32 
                radioWidth { cell._RadioButton ? cell._RadioButton->GetSize().x : 0.0f },
                gap { (cell._RadioButton && cell._Label) ? cell._Label->GetTextPadding() : 0.0f },
                textWidth { cell._Label ? cell._Label->GetTextWidth() : 0.0f },
                combinedWidth { radioWidth + gap + textWidth };
            f32 combinedLeft { cell._Position.x - combinedWidth * 0.5f }; // Center (default)

            if(_gridContentAlignment == TextAlignment::Left)
                combinedLeft = cell._Position.x - cell._Size.x * 0.5f + _border->GetBorderWidth() + _borderPadding;
            else if(_gridContentAlignment == TextAlignment::Right)
                combinedLeft = cell._Position.x + cell._Size.x * 0.5f - combinedWidth + _border->GetBorderWidth() + _borderPadding;

            if(cell._RadioButton)
                cell._RadioButton->SetPosition(glm::vec3(combinedLeft + radioWidth * 0.5f, cell._Position.y, _position.z + 0.02f));

            if(cell._Label)
            {
                cell._Label->SetTextAlignment(TextAlignment::Center);
                cell._Label->SetPosition(glm::vec3(combinedLeft + radioWidth + gap + textWidth * 0.5f, cell._Position.y, _position.z + 0.03f));
            }

            cursorX += columnWidth;
        }

        cursorY -= rowHeight;
    }
}

void RetroFuturaGUI::RadioButtonGroup::SetGridContentPadding(const f32 pixels)
{
    _borderPadding = pixels;
    updateGridLayout();
}