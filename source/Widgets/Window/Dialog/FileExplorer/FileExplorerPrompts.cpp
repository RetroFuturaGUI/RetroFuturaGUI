#include "FileExplorerPrompts.hpp"
#include "Button.hpp"
#include "CheckBox.hpp"
#include "Label.hpp"
#include "MenuBar.hpp"
#include "PlatformBridge.hpp"
#include "TextBox.hpp"
#include "Window.hpp"
#include <format>

RetroFuturaGUI::FileExplorer::NamePrompt::NamePrompt(std::string_view title, std::string_view message, std::string_view name, const ViewFont& font)
    : Popup(std::format("FileExplorerPrompt{}", _promptCount), _width, _height)
{
    const uSize number { _promptCount++ };
    Window* window { GetWindow() };

    if(!window)
        return;

    styleWindow(*window, title);
    std::unique_ptr<ViewHost> host { std::make_unique<ViewHost>(std::format("FileExplorerPromptScene{}", number), window, _axis) };
    Lasagna* lasagna { host->GetLasagna() };

    if(!lasagna)
        return;

    Scene* scene { host->GetScene() };

    if(!scene)
        return;

    const glm::vec2 windowSize { static_cast<f32>(_width), static_cast<f32>(_height) };
    const ViewCell
        messageCell { ._Row = 1, ._Column = 0, ._ColumnSpan = 12, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        nameCell { ._Row = 2, ._Column = 0, ._ColumnSpan = 12, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        okCell { ._Row = 3, ._Column = 4, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        cancelCell { ._Row = 3, ._Column = 8, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };

    MenuBar* bar { attachBar(*host, *lasagna, title, 12, _axis._RowDefinition[0] * windowSize.y, windowSize, font) };
    Label* messageLabel { host->Attach<Label>("Message", *lasagna, messageCell) };
    _name = host->Attach<TextBox>("Name", *lasagna, nameCell);
    Button
        * ok { host->Attach<Button>("Ok", *lasagna, okCell) },
        * cancel { host->Attach<Button>("Cancel", *lasagna, cancelCell) };

    Button* close { bar ? bar->GetWidget<Button>(_barCloseIndex) : nullptr };

    if(!bar || !close || !messageLabel || !_name || !ok || !cancel)
        return;

    close->Connect_OnClick([this]() { Dismiss(); }, false);

    styleLabel(*messageLabel, message, cellSize(_axis, windowSize, messageCell), font);
    styleTextBox(*_name, "Name", cellSize(_axis, windowSize, nameCell), font);
    _name->SetValue<std::string_view>(name, false);
    _name->Connect_OnEnterPressed([this]() { confirm(); }, false);
    styleButton(*ok, "OK", cellSize(_axis, windowSize, okCell), font);
    ok->Connect_OnClick([this]() { confirm(); }, false);
    styleButton(*cancel, "Cancel", cellSize(_axis, windowSize, cancelCell), font);
    cancel->Connect_OnClick([this]() { Dismiss(); }, false);

    scene->Connect_OnUpdate([this]() { update(); }, false);
    AddScene(std::move(host));
}

void RetroFuturaGUI::FileExplorer::NamePrompt::TakeKeys()
{
    _escapeKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_escapeKey._Key);

    if(!_name)
        return;

    _name->TakeKeyboardFocus();
}

void RetroFuturaGUI::FileExplorer::NamePrompt::update()
{
    if(!wasPressed(_escapeKey))
        return;

    Dismiss();
}

void RetroFuturaGUI::FileExplorer::NamePrompt::confirm()
{
    if(!_name)
        return;

    Close(_name->GetText());
}

RetroFuturaGUI::FileExplorer::ConfirmPrompt::ConfirmPrompt(std::string_view title, std::string_view message, std::string_view detail, std::string_view confirmText, const ViewFont& font)
    : Popup(std::format("FileExplorerPrompt{}", _promptCount), _width, _height)
{
    const uSize number { _promptCount++ };
    Window* window { GetWindow() };

    if(!window)
        return;

    styleWindow(*window, title);
    std::unique_ptr<ViewHost> host { std::make_unique<ViewHost>(std::format("FileExplorerPromptScene{}", number), window, _axis) };
    Lasagna* lasagna { host->GetLasagna() };

    if(!lasagna)
        return;

    Scene* scene { host->GetScene() };

    if(!scene)
        return;

    const glm::vec2 windowSize { static_cast<f32>(_width), static_cast<f32>(_height) };
    const ViewCell
        messageCell { ._Row = 1, ._Column = 0, ._ColumnSpan = 12, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        detailCell { ._Row = 2, ._Column = 0, ._ColumnSpan = 12, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        confirmCell { ._Row = 3, ._Column = 4, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        cancelCell { ._Row = 3, ._Column = 8, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };

    MenuBar* bar { attachBar(*host, *lasagna, title, 12, _axis._RowDefinition[0] * windowSize.y, windowSize, font) };
    Label
        * messageLabel { host->Attach<Label>("Message", *lasagna, messageCell) },
        * detailLabel { host->Attach<Label>("Detail", *lasagna, detailCell) };
    Button
        * confirmButton { host->Attach<Button>("Confirm", *lasagna, confirmCell) },
        * cancel { host->Attach<Button>("Cancel", *lasagna, cancelCell) };

    Button* close { bar ? bar->GetWidget<Button>(_barCloseIndex) : nullptr };

    if(!bar || !close || !messageLabel || !detailLabel || !confirmButton || !cancel)
        return;

    close->Connect_OnClick([this]() { Dismiss(); }, false);

    styleLabel(*messageLabel, message, cellSize(_axis, windowSize, messageCell), font);
    styleLabel(*detailLabel, detail, cellSize(_axis, windowSize, detailCell), font);
    detailLabel->SetTextColors(std::array { glm::vec4(0.75f, 0.75f, 0.8f, 1.0f) }, ColorState::Enabled);
    styleButton(*confirmButton, confirmText, cellSize(_axis, windowSize, confirmCell), font);
    confirmButton->Connect_OnClick([this]() { Close(); }, false);
    styleButton(*cancel, "Cancel", cellSize(_axis, windowSize, cancelCell), font);
    cancel->Connect_OnClick([this]() { Dismiss(); }, false);

    scene->Connect_OnUpdate([this]() { update(); }, false);
    AddScene(std::move(host));
}

void RetroFuturaGUI::FileExplorer::ConfirmPrompt::TakeKeys()
{
    _enterKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_enterKey._Key);
    _escapeKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_escapeKey._Key);

    Window* window { GetWindow() };

    if(!window)
        return;

    window->TakeKeyboardInput();
}

void RetroFuturaGUI::FileExplorer::ConfirmPrompt::update()
{
    //Both taken every frame, so neither fires late
    const bool
        isEnterPressed { wasPressed(_enterKey) },
        isEscapePressed { wasPressed(_escapeKey) };

    if(isEscapePressed)
    {
        Dismiss();
        return;
    }

    if(!isEnterPressed)
        return;

    Close();
}

RetroFuturaGUI::FileExplorer::ClashPrompt::ClashPrompt(std::string_view name, std::string_view folder, const ViewFont& font)
    : Popup(std::format("FileExplorerPrompt{}", _promptCount), _width, _height)
{
    const uSize number { _promptCount++ };
    Window* window { GetWindow() };

    if(!window)
        return;

    styleWindow(*window, "Name taken");
    std::unique_ptr<ViewHost> host { std::make_unique<ViewHost>(std::format("FileExplorerPromptScene{}", number), window, _axis) };
    Lasagna* lasagna { host->GetLasagna() };

    if(!lasagna)
        return;

    Scene* scene { host->GetScene() };

    if(!scene)
        return;

    const glm::vec2 windowSize { static_cast<f32>(_width), static_cast<f32>(_height) };
    const ViewCell
        messageCell { ._Row = 1, ._Column = 0, ._ColumnSpan = 12, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        checkCell { ._Row = 2, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        checkLabelCell { ._Row = 2, ._Column = 1, ._ColumnSpan = 11, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        replaceCell { ._Row = 3, ._Column = 0, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        skipCell { ._Row = 3, ._Column = 4, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        keepBothCell { ._Row = 3, ._Column = 8, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };

    MenuBar* bar { attachBar(*host, *lasagna, "Name taken", 12, _axis._RowDefinition[0] * windowSize.y, windowSize, font) };
    Label
        * messageLabel { host->Attach<Label>("Message", *lasagna, messageCell) },
        * checkLabel { host->Attach<Label>("ForRemainingLabel", *lasagna, checkLabelCell) };
    _forRemaining = host->Attach<CheckBox>("ForRemaining", *lasagna, checkCell);
    Button
        * replace { host->Attach<Button>("Replace", *lasagna, replaceCell) },
        * skip { host->Attach<Button>("Skip", *lasagna, skipCell) },
        * keepBoth { host->Attach<Button>("KeepBoth", *lasagna, keepBothCell) };

    Button* close { bar ? bar->GetWidget<Button>(_barCloseIndex) : nullptr };

    if(!bar || !close || !messageLabel || !checkLabel || !_forRemaining || !replace || !skip || !keepBoth)
        return;

    close->Connect_OnClick([this]() { Dismiss(); }, false);

    styleLabel(*messageLabel, std::format("{} is already in {}.", name, folder), cellSize(_axis, windowSize, messageCell), font);
    styleLabel(*checkLabel, "Do the same for the remaining clashes", cellSize(_axis, windowSize, checkLabelCell), font);

    //A square in its cell
    const glm::vec3 checkSize { cellSize(_axis, windowSize, checkCell) };
    styleCheckBox(*_forRemaining, checkSize.x < checkSize.y ? checkSize.x : checkSize.y);
    styleButton(*replace, "Replace", cellSize(_axis, windowSize, replaceCell), font);
    replace->Connect_OnClick([this]() { answer(ClashAnswer::Replace); }, false);
    styleButton(*skip, "Skip", cellSize(_axis, windowSize, skipCell), font);
    skip->Connect_OnClick([this]() { answer(ClashAnswer::Skip); }, false);
    styleButton(*keepBoth, "Keep both", cellSize(_axis, windowSize, keepBothCell), font);
    keepBoth->Connect_OnClick([this]() { answer(ClashAnswer::KeepBoth); }, false);

    scene->Connect_OnUpdate([this]() { update(); }, false);
    AddScene(std::move(host));
}

void RetroFuturaGUI::FileExplorer::ClashPrompt::TakeKeys()
{
    _enterKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_enterKey._Key);
    _escapeKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_escapeKey._Key);

    Window* window { GetWindow() };

    if(!window)
        return;

    window->TakeKeyboardInput();
}

void RetroFuturaGUI::FileExplorer::ClashPrompt::update()
{
    //Both taken every frame, so neither fires late
    const bool
        isEnterPressed { wasPressed(_enterKey) },
        isEscapePressed { wasPressed(_escapeKey) };

    if(isEscapePressed)
    {
        Dismiss();
        return;
    }

    if(!isEnterPressed)
        return;

    answer(ClashAnswer::KeepBoth); //the answer that loses nothing
}

void RetroFuturaGUI::FileExplorer::ClashPrompt::answer(const ClashAnswer clashAnswer)
{
    const bool isForRemaining { _forRemaining && _forRemaining->GetValue() };
    Close(clashAnswer, isForRemaining ? ClashScope::RemainingItems : ClashScope::ThisItem);
}