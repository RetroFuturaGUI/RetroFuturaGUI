#include "RadioButtonGroup.hpp"
#include "ITextProperties.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "RadioButton.hpp"
#include "config.hpp"
#include <algorithm>
#include <cstddef>
#include <glm/ext/vector_int2_sized.hpp>
#include #include <print>
#include <string>
#include <utility>
<memory>

RetroFuturaGUI::RadioButtonGroup::RadioButtonGroup(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::RadioButtonGroup;
    _border = std::make_unique<Frame>(projection);
    _text = std::make_unique<Text>(projection);

    if(!_border)
        return;

}

void RetroFuturaGUI::RadioButtonGroup::Draw()
{
    _removedWidgets.clear(); //no slot of theirs runs between frames

    if(!_text)
        return;

    drawBorder();

    _text->Draw();

    for(const auto& row : _grid)
        for(const auto& column: row)
        {
            if(!column._RadioButton)
                continue;

            column._RadioButton->UpdateTooltip();
            column._RadioButton->Draw();

            if(!column._Label)
                continue;

            column._Label->UpdateTooltip();
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
    if(!_border)
        return;

    if(!_text)
        return;

    IWidget::SetRotation(rotation);

    _border->SetRotation(rotation);
    _text->SetRotation(rotation);
}

void RetroFuturaGUI::RadioButtonGroup::DeselectOthers(RadioButton* activeRadioButton)
{
    for(auto& row : _grid)
        for(auto& column : row)
        {
            if(!column._RadioButton)
                continue;
            
            if(column._RadioButton.get() != activeRadioButton)
                column._RadioButton->SetValue(false, false);
        }
}

RetroFuturaGUI::RadioButton* RetroFuturaGUI::RadioButtonGroup::AddRadioButton(std::string_view name, const glm::i64vec2& index)
{
    if(0 > index.x || 0 > index.y || static_cast<uSize>(index.x) >= _grid.size() || static_cast<uSize>(index.y) >= _grid[static_cast<uSize>(index.x)].size())
    {
        std::println("ERROR::RADIOBUTTONGROUP::CELL_OUT_OF_RANGE::{}::{}::[{},{}]", _name, name, index.x, index.y);
        return nullptr;
    }

    GridCell& cell { _grid[static_cast<uSize>(index.x)][static_cast<uSize>(index.y)] };

    if(cell._RadioButton)
    {
        std::println("ERROR::RADIOBUTTONGROUP::CELL_TAKEN::{}::{}::[{},{}]", _name, name, index.x, index.y);
        return nullptr;
    }

    if(findCell(name))
    {
        std::println("ERROR::RADIOBUTTONGROUP::RADIOBUTTON_NAME_TAKEN::{}::{}", _name, name);
        return nullptr;
    }

    //Most widget constructors take the name as const std::string&, so it is materialised here
    const std::string radioButtonName { name };
    cell._RadioButton = std::make_unique<RadioButton>(radioButtonName, &_projection, this, _widgetTypeID, _parentWindow, this);
    cell._Label = std::make_unique<Label>(radioButtonName + "Label", &_projection, this, _widgetTypeID, _parentWindow);
    updateGridLayout();
    return cell._RadioButton.get();
}

void RetroFuturaGUI::RadioButtonGroup::SetLabel(std::string_view name, std::string_view label, const bool emitSignal)
{
    GridCell* cell { findCell(name) };

    if(!cell)
    {
        std::println("ERROR::RADIOBUTTONGROUP::NO_CELL_FOUND::{}::{}", _name, name);
        return;
    }

    cell->_Label->SetText(label, emitSignal);
    updateGridLayout();
}

bool RetroFuturaGUI::RadioButtonGroup::RemoveRadioButton(std::string_view name)
{
    GridCell* cell { findCell(name) };

    if(!cell)
        return false;

    releaseCell(*cell);
    return true;
}

RetroFuturaGUI::RadioButton* RetroFuturaGUI::RadioButtonGroup::GetRadioButton(std::string_view name) const
{
    const GridCell* cell { findCell(name) };

    if(!cell)
        return nullptr;

    return cell->_RadioButton.get();
}

RetroFuturaGUI::Label* RetroFuturaGUI::RadioButtonGroup::GetRadioButtonLabel(std::string_view radioButtonName) const
{
    const GridCell* cell { findCell(radioButtonName) };

    if(!cell)
        return nullptr;

    return cell->_Label.get();
}

RetroFuturaGUI::RadioButtonGroup::GridCell* RetroFuturaGUI::RadioButtonGroup::findCell(std::string_view name)
{
    return const_cast<GridCell*>(std::as_const(*this).findCell(name));
}

const RetroFuturaGUI::RadioButtonGroup::GridCell* RetroFuturaGUI::RadioButtonGroup::findCell(std::string_view name) const
{
    for(const auto& row : _grid)
        for(const GridCell& cell : row)
            if(cell._RadioButton && name == cell._RadioButton->GetName())
                return &cell;

    return nullptr;
}

void RetroFuturaGUI::RadioButtonGroup::releaseCell(GridCell& cell)
{
    //Kept alive until the next Draw, since one of the button's own slots may be running right now
    if(cell._RadioButton)
        _removedWidgets.push_back(std::move(cell._RadioButton));

    if(cell._Label)
        _removedWidgets.push_back(std::move(cell._Label));
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
        ._Edge = BorderEdge::Top,
        ._Offset = _textLeftPadding,
        ._Length = textWidth + textPadding * 2.0f,
        ._AnchorFarCorner = false,
        ._Repeat = 1
    };

    IBorder::SetBorderGaps(std::span<BorderGap>(&textGap, 1));
    _text->SetTextAlignment(TextAlignment::Center);
    _text->SetPosition(glm::vec3(
        newPos.x - newSize.x * 0.5f + _textLeftPadding + textGap._Length * 0.5f,
        newPos.y + newSize.y * 0.5f,
        newPos.z + 0.01f)
    );
}

void RetroFuturaGUI::RadioButtonGroup::SetBorderGaps(std::span<BorderGap> gaps)
{
    IBorder::SetBorderGaps(gaps);

    for(const BorderGap& gap : gaps)
        if(gap._Edge == BorderEdge::Left)
        {
            _textLeftPadding = gap._Offset;
            return;
        }
}

void RetroFuturaGUI::RadioButtonGroup::SetAxisDefinitions(std::span<f32> rowDefinitions, std::span<f32> columnDefinitions)
{
    _rowDefinitions.assign(rowDefinitions.begin(), rowDefinitions.end());
    _columnDefinitions.assign(columnDefinitions.begin(), columnDefinitions.end());

    //Cells the new grid still has keep their buttons; the others give theirs up
    for(uSize row { 0 }; row < _grid.size(); ++row)
        for(uSize column { 0 }; column < _grid[row].size(); ++column)
            if(row >= _rowDefinitions.size() || column >= _columnDefinitions.size())
                releaseCell(_grid[row][column]);

    _grid.resize(_rowDefinitions.size());

    for(std::vector<GridCell>& row : _grid)
        row.resize(_columnDefinitions.size());

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