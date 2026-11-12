#include "MessagePopup.hpp"
#include "Button.hpp"
#include "FontManager.hpp"
#include "Label.hpp"
#include "MenuBar.hpp"
#include "PlatformBridge.hpp"
#include "Scene.hpp"
#include "StylingManager.hpp"
#include "Window.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <format>
#include <memory>
#include <vector>

RetroFuturaGUI::MessagePopup::MessagePopup(const PopupTypes type, std::string_view summary, std::string_view description)
    : Popup(std::format("MessagePopup{}", _popupCount), static_cast<i32>(_width), static_cast<i32>(_barHeight + _summaryHeight + _shortestDescription + _buttonRowHeight), scopeOf(type)),
      _type(shownType(type)),
      _enterPressCountSeen(PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN)) //the Enter that opened it, if one did, is spent
{
    const uSize number { _popupCount++ };
    addMissingEntries();
    Window* window { GetWindow() };

    if(!window)
        return;

    styleWindow(*window, titleOf(_type));

    //Laid out again once the description is measured, so the window is as tall as its lines
    const std::vector<f32> columns(_columnCount, 1.0f / static_cast<f32>(_columnCount));
    const AxisDefinition firstAxis { ._RowDefinition = { 0.25f, 0.25f, 0.25f, 0.25f }, ._ColumnDefinition = columns, ._LayerDefinition = { 1.0f } };
    std::unique_ptr<ViewHost> host { std::make_unique<ViewHost>(std::format("MessagePopupScene{}", number), window, firstAxis) };
    Lasagna* lasagna { host->GetLasagna() };

    if(!lasagna)
        return;

    Scene* scene { host->GetScene() };

    if(!scene)
        return;

    const ViewFont
        font { popupFont(PlatformBridge::Fonts::Weight::Normal) },
        summaryFont { popupFont(PlatformBridge::Fonts::Weight::Bold) };
    const ViewCell
        iconCell { ._Row = 1, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        summaryCell { ._Row = 1, ._Column = 1, ._ColumnSpan = _columnCount - 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        okCell { ._Row = 3, ._Column = _columnCount - 3, ._ColumnSpan = 3, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    Label* descriptionLabel { host->Attach<Label>("Description", *lasagna, _descriptionCell) };

    if(!descriptionLabel)
        return;

    //Wrapped at the width the margins leave; its lines, padded, make its row.
    styleLabel(*descriptionLabel, "", glm::vec3(_width - 2.0f * _margin, _shortestDescription, 0.01f), font);
    descriptionLabel->GetText().EnableWordWrap(true);
    descriptionLabel->GetText().GetElement().SetAnchor(TextAnchor::Top);
    descriptionLabel->GetText().SetString(description, false);
    _description = descriptionLabel;
    _lasagna = lasagna;
    layoutToDescription(); //still hidden. The stack centers it at this size once it is pushed
    const glm::vec2 windowSize { _width, heightFor(_descriptionHeight) };
    const AxisDefinition axis { axisFor(_descriptionHeight) };

    MenuBar* bar { attachBar(*host, *lasagna, titleOf(_type), _columnCount, _barHeight, windowSize, font) };
    Button* close { bar ? bar->GetWidget<Button>(_barCloseIndex) : nullptr };
    Label* barTitle { bar ? bar->GetWidget<Label>(0) : nullptr };
    Button
        * icon { host->Attach<Button>("Icon", *lasagna, iconCell) },
        * ok { host->Attach<Button>("Ok", *lasagna, okCell) };
    Label* summaryLabel { host->Attach<Label>("Summary", *lasagna, summaryCell) };

    if(!bar || !close || !barTitle || !icon || !ok || !summaryLabel)
        return;

    barTitle->SetStyleAccessors(StyleAccessor::Title); //the kind's accent, from its scope
    close->Connect_OnClick([this]() { Dismiss(); }, false);

    //A button that never takes the mouse, showing only its background: the image of its scope's Button:icon entry
    icon->SetSize(glm::vec3(_iconSide, _iconSide, 0.01f));
    icon->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f) }, ColorState::Enabled);
    icon->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f) }, ColorState::Disabled);
    icon->GetBorder().SetBuiltInWidth(styleKey(), 0.0f);
    icon->SetEnabled(false, false);
    icon->SetStyleAccessors(StyleAccessor::Icon);

    styleLabel(*summaryLabel, summary, cellSize(axis, windowSize, summaryCell), summaryFont);
    styleButton(*ok, "OK", cellSize(axis, windowSize, okCell), font);
    ok->Connect_OnClick([this]() { Close(); }, false);

    scene->Connect_OnInteract([this]() { handleKeys(); }, false);
    scene->Connect_OnUpdate([this]() { layoutToDescription(); }, false); //a font a scope or flavor changed since makes the lines taller or shorter
    AddScene(std::move(host));
}

RetroFuturaGUI::PopupTypes RetroFuturaGUI::MessagePopup::GetType() const
{
    return _type;
}

void RetroFuturaGUI::MessagePopup::handleKeys()
{
    const u32 enterPressCount { PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN) };

    if(enterPressCount == _enterPressCountSeen)
        return;

    _enterPressCountSeen = enterPressCount;
    Close();
}

void RetroFuturaGUI::MessagePopup::layoutToDescription()
{
    if(!_description)
        return;

    if(!_lasagna)
        return;

    Window* window { GetWindow() };

    if(!window)
        return;

    const f32
        linesHeight { _description->GetText().GetElement().GetSpanHeight() + 2.0f * _description->GetText().GetPadding() },
        descriptionHeight { std::ceil(std::max(linesHeight, _shortestDescription) + 2.0f * _margin) };

    if(descriptionHeight == _descriptionHeight)
        return;

    _descriptionHeight = descriptionHeight;
    const f32 height { heightFor(descriptionHeight) };
    const AxisDefinition axis { axisFor(descriptionHeight) };
    window->SetWindowSize(static_cast<i32>(_width), static_cast<i32>(height));
    _lasagna->SetAxisDefinition(axis);
    _description->SetSize(cellSize(axis, glm::vec2(_width, height), _descriptionCell));
}

f32 RetroFuturaGUI::MessagePopup::heightFor(const f32 descriptionHeight)
{
    return _barHeight + _summaryHeight + descriptionHeight + _buttonRowHeight;
}

RetroFuturaGUI::AxisDefinition RetroFuturaGUI::MessagePopup::axisFor(const f32 descriptionHeight)
{
    const f32 height { heightFor(descriptionHeight) };
    return AxisDefinition
    {
        ._RowDefinition = { _barHeight / height, _summaryHeight / height, descriptionHeight / height, _buttonRowHeight / height },
        ._ColumnDefinition = std::vector<f32>(_columnCount, 1.0f / static_cast<f32>(_columnCount)),
        ._LayerDefinition = { 1.0f }
    };
}

RetroFuturaGUI::StyleScope RetroFuturaGUI::MessagePopup::scopeOf(const PopupTypes type)
{
    switch(shownType(type))
    {
        case PopupTypes::Warning:
            return StyleScope::Warning;
        case PopupTypes::Error:
            return StyleScope::Error;
        default:
            return StyleScope::Message;
    }
}

RetroFuturaGUI::PopupTypes RetroFuturaGUI::MessagePopup::shownType(const PopupTypes type)
{
    if(PopupTypes::Warning == type || PopupTypes::Error == type)
        return type;

    return PopupTypes::Message;
}

std::string_view RetroFuturaGUI::MessagePopup::titleOf(const PopupTypes type)
{
    switch(shownType(type))
    {
        case PopupTypes::Warning:
            return "Warning";
        case PopupTypes::Error:
            return "Error";
        default:
            return "Information";
    }
}