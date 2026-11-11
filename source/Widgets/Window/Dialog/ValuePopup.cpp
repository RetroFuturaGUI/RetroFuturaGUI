#include "ValuePopup.hpp"
#include "Button.hpp"
#include "ColorPreview.hpp"
#include "MenuBar.hpp"
#include "PlatformBridge.hpp"
#include "Scene.hpp"
#include "StylingManager.hpp"
#include <algorithm>
#include <cmath>

RetroFuturaGUI::ValueForm::ValueForm(Window* window, const uSize number, std::string_view title, std::string_view description, std::span<const ValueRowKind> kinds, std::span<const std::string_view> labels, std::span<const uSize> listItemCounts)
    : _enterPressCountSeen(PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN)) //the Enter that opened it, if one did, is spent.
{
    addMissingFlavors();

    if(!window)
        return;

    styleWindow(*window, title);

    //Laid out again once the description is measured. Bar, description, one row per field, buttons
    const uSize fieldCount { std::min(kinds.size(), labels.size()) };
    const std::vector<f32> columns(_columnCount, 1.0f / static_cast<f32>(_columnCount));
    const AxisDefinition firstAxis { ._RowDefinition = std::vector<f32>(fieldCount + 3, 1.0f / static_cast<f32>(fieldCount + 3)), ._ColumnDefinition = columns, ._LayerDefinition = { 1.0f } };
    _host = std::make_unique<ViewHost>(std::format("ValuePopupScene{}", number), window, firstAxis);
    Lasagna* lasagna { _host->GetLasagna() };

    if(!lasagna)
        return;

    const ViewFont font { popupFont(PlatformBridge::Fonts::Weight::Normal) };
    const ViewCell descriptionCell { ._Row = 1, ._Column = 0, ._ColumnSpan = _columnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    Label* descriptionLabel { _host->Attach<Label>("Description", *lasagna, descriptionCell) };

    if(!descriptionLabel)
        return;

    //Wrapped at the width the margins leave. Its lines, padded, make its row.
    styleLabel(*descriptionLabel, "", glm::vec3(_width - 2.0f * _margin, _emptyDescription, 0.01f), font);
    descriptionLabel->GetText().EnableWordWrap(true);
    descriptionLabel->GetText().GetElement().SetAnchor(TextAnchor::Top);
    descriptionLabel->GetText().SetString(description, false);
    const f32 descriptionHeight
    {
        description.empty()
            ? _emptyDescription
            : std::ceil(descriptionLabel->GetText().GetElement().GetSpanHeight() + 2.0f * descriptionLabel->GetText().GetPadding() + 2.0f * _margin)
    };
    std::vector<f32> rowHeights { _barHeight, descriptionHeight };

    for(uSize field = 0; field < fieldCount; ++field)
        rowHeights.push_back(rowHeightOf(kinds[field]));

    //A choice's list opens on a side of its row where its items fit. What neither side has goes in above the buttons
    const f32 listItemHeight { _fieldHeight - 2.0f * _margin }; //as tall as the box
    f32 spacerHeight { 0.0f };

    for(uSize field = 0; field < fieldCount && field < listItemCounts.size(); ++field)
    {
        if(0 == listItemCounts[field])
            continue;

        const f32 neededHeight { static_cast<f32>(std::min(listItemCounts[field], _listRowCount)) * listItemHeight };
        f32
            above { _margin },
            below { _margin + _buttonRowHeight };

        for(uSize track = 1; track < 2 + field; ++track)
            above += rowHeights[track];

        for(uSize track = 3 + field; track < rowHeights.size(); ++track)
            below += rowHeights[track];

        if(neededHeight <= above || neededHeight <= below)
            continue;

        spacerHeight = std::max(spacerHeight, neededHeight - below);
    }

    if(0.0f < spacerHeight)
        rowHeights.push_back(std::ceil(spacerHeight));

    rowHeights.push_back(_buttonRowHeight);
    const uSize buttonRow { rowHeights.size() - 1 };
    f32 height { 0.0f };

    for(const f32 rowHeight : rowHeights)
        height += rowHeight;

    std::vector<f32> rowShares {};

    for(const f32 rowHeight : rowHeights)
        rowShares.push_back(rowHeight / height);

    const glm::vec2 windowSize { _width, height };
    const AxisDefinition axis { ._RowDefinition = rowShares, ._ColumnDefinition = columns, ._LayerDefinition = { 1.0f } };

    //Still hidden. The stack centers it at this size once it is pushed
    window->SetWindowSize(static_cast<i32>(_width), static_cast<i32>(height));
    lasagna->SetAxisDefinition(axis);
    descriptionLabel->SetSize(cellSize(axis, windowSize, descriptionCell));

    MenuBar* bar { attachBar(*_host, *lasagna, title, _columnCount, _barHeight, windowSize, font) };
    _close = bar ? bar->GetWidget<Button>(_barCloseIndex) : nullptr;

    for(uSize field = 0; field < fieldCount; ++field)
        attachRow(*lasagna, 2 + field, kinds[field], labels[field], axis, windowSize);

    const ViewCell
        okCell { ._Row = buttonRow, ._Column = _columnCount - 6, ._ColumnSpan = 3, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        cancelCell { ._Row = buttonRow, ._Column = _columnCount - 3, ._ColumnSpan = 3, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    _ok = _host->Attach<Button>("Ok", *lasagna, okCell);
    _cancel = _host->Attach<Button>("Cancel", *lasagna, cancelCell);

    if(_ok)
        styleButton(*_ok, "OK", cellSize(axis, windowSize, okCell), font);

    if(_cancel)
        styleButton(*_cancel, "Cancel", cellSize(axis, windowSize, cancelCell), font);

    //Typing goes to the first text field from the start
    for(const Row& row : _rows)
    {
        if(!row._TextBox)
            continue;

        row._TextBox->TakeKeyboardFocus();
        break;
    }
}

void RetroFuturaGUI::ValueForm::attachRow(Lasagna& lasagna, const uSize lasagnaRow, const ValueRowKind kind, std::string_view label, const AxisDefinition& axis, const glm::vec2& windowSize)
{
    const uSize row { _rows.size() };
    _rows.push_back(Row { ._Kind = kind, ._TextBox = nullptr, ._CheckBox = nullptr, ._ColorPicker = nullptr, ._Slider = nullptr, ._Brightness = nullptr, ._Alpha = nullptr, ._Preview = nullptr, ._RangeLabel = nullptr, ._ComboBox = nullptr, ._SpaceAbove = 0.0f, ._SpaceBelow = 0.0f, ._IsInvalid = false });
    const ViewFont font { popupFont(PlatformBridge::Fonts::Weight::Normal) };
    const uSize inputColumnCount { _columnCount - _labelColumnCount };
    const ViewCell
        labelCell { ._Row = lasagnaRow, ._Column = 0, ._ColumnSpan = _labelColumnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        inputCell { ._Row = lasagnaRow, ._Column = _labelColumnCount, ._ColumnSpan = inputColumnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };

    if(Label* fieldLabel { _host->Attach<Label>(std::format("Label{}", row), lasagna, labelCell) })
        styleLabel(*fieldLabel, label, cellSize(axis, windowSize, labelCell), font);

    switch(kind)
    {
        case ValueRowKind::Text:
        case ValueRowKind::Number:
        {
            _rows[row]._TextBox = _host->Attach<TextBox>(std::format("Field{}", row), lasagna, inputCell);

            if(!_rows[row]._TextBox)
                return;

            styleTextBox(*_rows[row]._TextBox, "", cellSize(axis, windowSize, inputCell), font);
        } break;
        case ValueRowKind::Check:
        {
            //A square at the start of the field, not centered across it
            const ViewCell checkCell { ._Row = lasagnaRow, ._Column = _labelColumnCount, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
            const glm::vec3 checkSize { cellSize(axis, windowSize, checkCell) };
            _rows[row]._CheckBox = _host->Attach<CheckBox>(std::format("Field{}", row), lasagna, checkCell);

            if(!_rows[row]._CheckBox)
                return;

            styleCheckBox(*_rows[row]._CheckBox, std::min(checkSize.x, checkSize.y));
        } break;
        case ValueRowKind::Range:
        {
            const ViewCell
                sliderCell { ._Row = lasagnaRow, ._Column = _labelColumnCount, ._ColumnSpan = inputColumnCount - 2, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
                numberCell { ._Row = lasagnaRow, ._Column = _columnCount - 2, ._ColumnSpan = 2, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
            _rows[row]._Slider = _host->Attach<Slider>(std::format("Field{}", row), lasagna, sliderCell);
            _rows[row]._RangeLabel = _host->Attach<Label>(std::format("Number{}", row), lasagna, numberCell);

            if(!_rows[row]._Slider || !_rows[row]._RangeLabel)
                return;

            const glm::vec3 sliderSize { cellSize(axis, windowSize, sliderCell) };
            styleSlider(*_rows[row]._Slider);
            _rows[row]._Slider->SetSize(glm::vec3(sliderSize.x, std::min(sliderSize.y, 20.0f), 0.01f)); //a thin track, centered in the row
            styleLabel(*_rows[row]._RangeLabel, "", cellSize(axis, windowSize, numberCell), font);
        } break;
        case ValueRowKind::Choice:
        {
            //The room for its list on either side, the bar left out. FitChoiceList picks the side once the items are known
            for(uSize track = 1; track < lasagnaRow; ++track)
                _rows[row]._SpaceAbove += axis._RowDefinition[track] * windowSize.y;

            for(uSize track = lasagnaRow + 1; track < axis._RowDefinition.size(); ++track)
                _rows[row]._SpaceBelow += axis._RowDefinition[track] * windowSize.y;

            _rows[row]._SpaceAbove += _margin;
            _rows[row]._SpaceBelow += _margin;
            _rows[row]._ComboBox = _host->Attach<ComboBox>(std::format("Field{}", row), lasagna, inputCell);

            if(!_rows[row]._ComboBox)
                return;

            ComboBox& comboBox { *_rows[row]._ComboBox };
            comboBox.SetFontFamily(font._Family, font._Size, FontSizeUnit::Pixels, font._Slant, font._Weight);
            comboBox.SetSize(cellSize(axis, windowSize, inputCell));
            comboBox.SetTextAlignment(TextAlignment::Left);
            comboBox.GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) }, ColorState::Enabled);
            comboBox.GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.3f, 0.3f, 0.3f, 1.0f) }, ColorState::Hover);
            comboBox.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(8.0f));
            comboBox.GetBorder().SetBuiltInCornerRadii(styleKey(), glm::vec4(8.0f));
            comboBox.GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Enabled);
            comboBox.GetBorder().SetBuiltInWidth(styleKey(), 1.0f);
            //The open list shows its box's state, hovered or clicked while it is open.
            for(const ColorState state : { ColorState::Enabled, ColorState::Hover, ColorState::Clicked })
            {
                comboBox.GetList().GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.14f, 0.14f, 0.18f, 1.0f) }, state);
                comboBox.GetList().GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, state);
                comboBox.GetList().GetHighlight().SetBuiltInColors(styleKey(), std::array { _toggleOnColor }, state);
            }

        } break;
        case ValueRowKind::Color:
        {
            //The picker, a vertical slider each for brightness and alpha, and the color over the checkerboard beside it opaque
            const ViewCell
                pickerCell { ._Row = lasagnaRow, ._Column = _labelColumnCount, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
                brightnessCell { ._Row = lasagnaRow, ._Column = _labelColumnCount + 4, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
                alphaCell { ._Row = lasagnaRow, ._Column = _labelColumnCount + 5, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
                previewCell { ._Row = lasagnaRow, ._Column = _labelColumnCount + 6, ._ColumnSpan = 2, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
            Row& colorRow { _rows[row] };
            colorRow._ColorPicker = _host->Attach<ColorPicker>(std::format("Field{}", row), lasagna, pickerCell);
            colorRow._Brightness = _host->Attach<Slider>(std::format("Brightness{}", row), lasagna, brightnessCell);
            colorRow._Alpha = _host->Attach<Slider>(std::format("Alpha{}", row), lasagna, alphaCell);
            colorRow._Preview = _host->Attach<ColorPreview>(std::format("Preview{}", row), lasagna, previewCell);

            if(!colorRow._ColorPicker || !colorRow._Brightness || !colorRow._Alpha || !colorRow._Preview)
                return;

            const glm::vec3
                pickerSize { cellSize(axis, windowSize, pickerCell) },
                brightnessSize { cellSize(axis, windowSize, brightnessCell) },
                previewSize { cellSize(axis, windowSize, previewCell) };
            const f32 pickerSide { std::min(pickerSize.x, pickerSize.y) };
            colorRow._ColorPicker->SetSize(glm::vec3(pickerSide, pickerSide, 0.01f));
            colorRow._ColorPicker->GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Enabled);
            colorRow._ColorPicker->EnableAlpha(true, false);

            for(Slider* slider : { colorRow._Brightness, colorRow._Alpha })
            {
                styleSlider(*slider);
                slider->SetOrientation(IRangedValue::Orientation::Vertical);
                slider->SetSize(glm::vec3(std::min(brightnessSize.x, 20.0f), pickerSide, 0.01f));
                slider->SetMinValue<f32>(0.0f);
                slider->SetMaxValue<f32>(1.0f);
                slider->SetValue<f32>(1.0f, false);
                slider->Connect_OnValueChanged([this, row]() { followColor(row); }, false);
            }

            colorRow._Preview->SetSize(glm::vec3(previewSize.x, std::min(previewSize.y, 80.0f), 0.01f));
            colorRow._Preview->EnableDualPreview(true);
            colorRow._Preview->SetDualPreviewAlignment(ColorPreview::DualPreviewAlignment::Vertical);
            colorRow._ColorPicker->Connect_OnColorChanged([this, row]()
                {
                    Row& changed { _rows[row] };

                    if(!changed._ColorPicker || !changed._Preview)
                        return;

                    changed._Preview->SetPreviewColor(changed._ColorPicker->GetColorRGBAF());
                }, false);
        } break;
        default:
        break;
    }
}

void RetroFuturaGUI::ValueForm::FitChoiceList(const uSize row)
{
    if(_rows.size() <= row)
        return;

    const Row& choiceRow { _rows[row] };

    if(!choiceRow._ComboBox)
        return;

    //Each item is a row as tall as the box: below when they fit there, else above, else on the roomier side, the rest scrolled
    ComboBox& comboBox { *choiceRow._ComboBox };
    const f32 itemHeight { std::max(comboBox.GetSize().y, 1.0f) };
    const uSize
        wantedCount { std::min(comboBox.GetItemCount(), comboBox.GetMaxVisibleItemCount()) },
        belowCount { static_cast<uSize>(choiceRow._SpaceBelow / itemHeight) },
        aboveCount { static_cast<uSize>(choiceRow._SpaceAbove / itemHeight) };
    const bool opensBelow { belowCount >= wantedCount || belowCount >= aboveCount };
    comboBox.SetDropDownPosition(opensBelow ? IDropDown::DropDownPosition::Bottom : IDropDown::DropDownPosition::Top);
    comboBox.SetMaxVisibleItemCount(std::max<uSize>(std::min(wantedCount, opensBelow ? belowCount : aboveCount), 1));
}

void RetroFuturaGUI::ValueForm::followColor(const uSize row)
{
    if(_rows.size() <= row)
        return;

    Row& colorRow { _rows[row] };

    if(!colorRow._ColorPicker || !colorRow._Brightness || !colorRow._Alpha || !colorRow._Preview)
        return;

    colorRow._ColorPicker->SetBrightness(colorRow._Brightness->GetValue<f32>(), false);
    colorRow._ColorPicker->SetAlpha(colorRow._Alpha->GetValue<f32>(), false);
    colorRow._Preview->SetPreviewColor(colorRow._ColorPicker->GetColorRGBAF());
}

void RetroFuturaGUI::ValueForm::SetColor(const uSize row, const glm::vec4& color)
{
    if(_rows.size() <= row)
        return;

    Row& colorRow { _rows[row] };

    if(!colorRow._ColorPicker || !colorRow._Brightness || !colorRow._Alpha || !colorRow._Preview)
        return;

    colorRow._ColorPicker->SetColor(color, false);
    colorRow._Brightness->SetValue<f32>(colorRow._ColorPicker->GetBrightness(), false);
    colorRow._Alpha->SetValue<f32>(colorRow._ColorPicker->GetAlpha(), false);
    colorRow._Preview->SetPreviewColor(colorRow._ColorPicker->GetColorRGBAF());
}

RetroFuturaGUI::TextBox* RetroFuturaGUI::ValueForm::GetTextBox(const uSize row) const
{
    return _rows.size() > row ? _rows[row]._TextBox : nullptr;
}

RetroFuturaGUI::CheckBox* RetroFuturaGUI::ValueForm::GetCheckBox(const uSize row) const
{
    return _rows.size() > row ? _rows[row]._CheckBox : nullptr;
}

RetroFuturaGUI::ColorPicker* RetroFuturaGUI::ValueForm::GetColorPicker(const uSize row) const
{
    return _rows.size() > row ? _rows[row]._ColorPicker : nullptr;
}

RetroFuturaGUI::Slider* RetroFuturaGUI::ValueForm::GetSlider(const uSize row) const
{
    return _rows.size() > row ? _rows[row]._Slider : nullptr;
}

RetroFuturaGUI::Label* RetroFuturaGUI::ValueForm::GetRangeLabel(const uSize row) const
{
    return _rows.size() > row ? _rows[row]._RangeLabel : nullptr;
}

RetroFuturaGUI::ComboBox* RetroFuturaGUI::ValueForm::GetComboBox(const uSize row) const
{
    return _rows.size() > row ? _rows[row]._ComboBox : nullptr;
}

RetroFuturaGUI::Button* RetroFuturaGUI::ValueForm::GetOk() const
{
    return _ok;
}

RetroFuturaGUI::Button* RetroFuturaGUI::ValueForm::GetCancel() const
{
    return _cancel;
}

RetroFuturaGUI::Button* RetroFuturaGUI::ValueForm::GetClose() const
{
    return _close;
}

RetroFuturaGUI::Scene* RetroFuturaGUI::ValueForm::GetScene() const
{
    if(!_host)
        return nullptr;

    return _host->GetScene();
}

std::unique_ptr<RetroFuturaGUI::ISceneHost> RetroFuturaGUI::ValueForm::TakeHost()
{
    return std::move(_host);
}

void RetroFuturaGUI::ValueForm::MarkInvalid(const uSize row, const bool isInvalid)
{
    if(_rows.size() <= row)
        return;

    Row& marked { _rows[row] };

    if(marked._IsInvalid == isInvalid)
        return;

    marked._IsInvalid = isInvalid;

    if(!marked._TextBox)
        return;

    if(isInvalid)
        marked._TextBox->AddFlavor(_invalidFlavorName);
    else
        marked._TextBox->RemoveFlavor(_invalidFlavorName);
}

void RetroFuturaGUI::ValueForm::EnableOk(const bool enable)
{
    if(!_ok)
        return;

    if(_ok->IsEnabled() == enable)
        return;

    _ok->SetEnabled(enable, false);
}

bool RetroFuturaGUI::ValueForm::WasEnterPressed()
{
    const u32 enterPressCount { PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN) };

    if(enterPressCount == _enterPressCountSeen)
        return false;

    _enterPressCountSeen = enterPressCount;
    return true;
}

i32 RetroFuturaGUI::ValueForm::FirstHeight()
{
    return static_cast<i32>(_barHeight + _emptyDescription + _buttonRowHeight);
}

uSize RetroFuturaGUI::ValueForm::NextNumber()
{
    return _popupCount++;
}

void RetroFuturaGUI::ValueForm::styleSlider(Slider& slider)
{
    //The track stays dark under the mouse; the indicator lightens while hovered and more while dragged.
    for(const ColorState state : { ColorState::Enabled, ColorState::Hover, ColorState::Clicked })
    {
        slider.GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.12f, 0.12f, 0.15f, 1.0f) }, state);
        slider.GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, state);
    }

    slider.GetBorder().SetBuiltInWidth(styleKey(), 1.0f);
    slider.GetIndicator().GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.45f, 0.45f, 0.6f, 1.0f) }, ColorState::Enabled);
    slider.GetIndicator().GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.55f, 0.55f, 0.72f, 1.0f) }, ColorState::Hover);
    slider.GetIndicator().GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.65f, 0.65f, 0.85f, 1.0f) }, ColorState::Clicked);

    for(const ColorState state : { ColorState::Enabled, ColorState::Hover, ColorState::Clicked })
        slider.GetIndicator().GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.6f, 0.6f, 0.8f, 1.0f) }, state);
}

f32 RetroFuturaGUI::ValueForm::rowHeightOf(const ValueRowKind kind)
{
    return ValueRowKind::Color == kind ? _colorHeight : _fieldHeight;
}

void RetroFuturaGUI::ValueForm::addMissingFlavors()
{
    if(StylingManager::HasFlavor(_invalidFlavorName))
        return;

    //Hovered or clicked, an invalid field still shows it.
    Flavor& invalid { StylingManager::GetFlavor(_invalidFlavorName) };

    for(const ColorState state : { ColorState::Enabled, ColorState::Hover, ColorState::Clicked })
        invalid.GetBorder().SetColors(std::array { glm::vec4(1.0f, 0.35f, 0.35f, 1.0f) }, state);
}