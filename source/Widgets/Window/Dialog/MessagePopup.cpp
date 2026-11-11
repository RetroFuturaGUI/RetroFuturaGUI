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
    : Popup(std::format("MessagePopup{}", _popupCount), static_cast<i32>(_width), static_cast<i32>(_barHeight + _summaryHeight + _shortestDescription + _buttonRowHeight)),
      _type(shownType(type)),
      _enterPressCountSeen(PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN)) //the Enter that opened it, if one did, is spent.
{
    const uSize number { _popupCount++ };
    addMissingFlavors();
    Window* window { GetWindow() };

    if(!window)
        return;

    styleWindow(*window, titleOf(_type));
    window->AddFlavor(GetFlavorName(_type));

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
        descriptionCell { ._Row = 2, ._Column = 0, ._ColumnSpan = _columnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        okCell { ._Row = 3, ._Column = _columnCount - 3, ._ColumnSpan = 3, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    Label* descriptionLabel { host->Attach<Label>("Description", *lasagna, descriptionCell) };

    if(!descriptionLabel)
        return;

    //Wrapped at the width the margins leave; its lines, padded, make its row.
    styleLabel(*descriptionLabel, "", glm::vec3(_width - 2.0f * _margin, _shortestDescription, 0.01f), font);
    descriptionLabel->GetText().EnableWordWrap(true);
    descriptionLabel->GetText().GetElement().SetAnchor(TextAnchor::Top);
    descriptionLabel->GetText().SetString(description, false);
    const f32
        linesHeight { descriptionLabel->GetText().GetElement().GetSpanHeight() + 2.0f * descriptionLabel->GetText().GetPadding() },
        descriptionHeight { std::ceil(std::max(linesHeight, _shortestDescription) + 2.0f * _margin) },
        height { _barHeight + _summaryHeight + descriptionHeight + _buttonRowHeight };
    const glm::vec2 windowSize { _width, height };
    const AxisDefinition axis
    {
        ._RowDefinition = { _barHeight / height, _summaryHeight / height, descriptionHeight / height, _buttonRowHeight / height },
        ._ColumnDefinition = columns,
        ._LayerDefinition = { 1.0f }
    };

    //Still hidden: the stack centers it at this size once it is pushed.
    window->SetWindowSize(static_cast<i32>(_width), static_cast<i32>(height));
    lasagna->SetAxisDefinition(axis);
    descriptionLabel->SetSize(cellSize(axis, windowSize, descriptionCell));

    MenuBar* bar { attachBar(*host, *lasagna, titleOf(_type), _columnCount, _barHeight, windowSize, font) };
    Button* close { bar ? bar->GetWidget<Button>(_barCloseIndex) : nullptr };
    Label* barTitle { bar ? bar->GetWidget<Label>(0) : nullptr };
    Button
        * icon { host->Attach<Button>("Icon", *lasagna, iconCell) },
        * ok { host->Attach<Button>("Ok", *lasagna, okCell) };
    Label* summaryLabel { host->Attach<Label>("Summary", *lasagna, summaryCell) };

    if(!bar || !close || !barTitle || !icon || !ok || !summaryLabel)
        return;

    bar->AddFlavor(GetFlavorName(_type));
    barTitle->AddFlavor(GetFlavorName(_type));
    close->Connect_OnClick([this]() { Dismiss(); }, false);

    //A button that never takes the mouse, showing only its background: the icon flavor's image.
    icon->SetSize(glm::vec3(_iconSide, _iconSide, 0.01f));
    icon->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f) }, ColorState::Enabled);
    icon->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f) }, ColorState::Disabled);
    icon->GetBorder().SetBuiltInWidth(styleKey(), 0.0f);
    icon->SetEnabled(false, false);
    icon->AddFlavor(GetIconFlavorName(_type));

    styleLabel(*summaryLabel, summary, cellSize(axis, windowSize, summaryCell), summaryFont);
    styleButton(*ok, "OK", cellSize(axis, windowSize, okCell), font);
    ok->Connect_OnClick([this]() { Close(); }, false);

    scene->Connect_OnInteract([this]() { handleKeys(); }, false);
    AddScene(std::move(host));
}

RetroFuturaGUI::PopupTypes RetroFuturaGUI::MessagePopup::GetType() const
{
    return _type;
}

std::string_view RetroFuturaGUI::MessagePopup::GetFlavorName(const PopupTypes type)
{
    switch(shownType(type))
    {
        case PopupTypes::Warning:
            return "popupWarning";
        case PopupTypes::Error:
            return "popupError";
        default:
            return "popupInformation";
    }
}

std::string_view RetroFuturaGUI::MessagePopup::GetIconFlavorName(const PopupTypes type)
{
    switch(shownType(type))
    {
        case PopupTypes::Warning:
            return "popupWarningIcon";
        case PopupTypes::Error:
            return "popupErrorIcon";
        default:
            return "popupInformationIcon";
    }
}

void RetroFuturaGUI::MessagePopup::handleKeys()
{
    const u32 enterPressCount { PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN) };

    if(enterPressCount == _enterPressCountSeen)
        return;

    _enterPressCountSeen = enterPressCount;
    Close();
}

void RetroFuturaGUI::MessagePopup::addMissingFlavors()
{
    struct KindLook
    {
        PopupTypes _Type { PopupTypes::Message };
        glm::vec4 _Accent { 1.0f };
        std::string_view _Icon {};
    };

    const std::array looks
    {
        KindLook { ._Type = PopupTypes::Message, ._Accent = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f), ._Icon = "InformationIcon.svg" },
        KindLook { ._Type = PopupTypes::Warning, ._Accent = glm::vec4(1.0f, 0.75f, 0.2f, 1.0f), ._Icon = "WarningIcon.svg" },
        KindLook { ._Type = PopupTypes::Error, ._Accent = glm::vec4(1.0f, 0.35f, 0.35f, 1.0f), ._Icon = "ErrorIcon.svg" }
    };
    const std::filesystem::path images { PlatformBridge::Paths::GetExecutablePathFSPath().parent_path() / "Resources" / "img" };

    for(const KindLook& look : looks)
    {
        if(!StylingManager::HasFlavor(GetFlavorName(look._Type)))
        {
            Flavor& accent { StylingManager::GetFlavor(GetFlavorName(look._Type)) };
            accent.GetBorder().SetColors(std::array { look._Accent }, ColorState::Enabled);
            accent.GetText().SetColors(std::array { look._Accent }, ColorState::Enabled);
        }

        if(!StylingManager::HasFlavor(GetIconFlavorName(look._Type)))
            StylingManager::GetFlavor(GetIconFlavorName(look._Type)).GetBackground().SetImage(toUtf8(images / look._Icon)); //UTF-8, as Texture reads paths.
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