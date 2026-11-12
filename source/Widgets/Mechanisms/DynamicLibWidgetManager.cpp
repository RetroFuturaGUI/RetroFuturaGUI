#include "DynamicLibWidgetManager.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "Button.hpp"
#include "FileExplorer.hpp"
#include "Label.hpp"
#include "TextBox.hpp"
#include "WidgetRegistry.hpp"
#include "WidgetText.hpp"
#include "Window.hpp"
#include <memory>
#include <string>
#include <vector>

void RetroFuturaGUI::DynamicLibWidgetManager::ConnectSlot(const char *id, CallbackType callback, const i32 action, const bool async)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch (widgetTypeID)
    {
        case WidgetTypeID::Button:
            connectSlotToButton(widget, callback, action, async);
        break;
        case WidgetTypeID::Label:
            connectSlotToLabel(widget, callback, action, async);
        break;
        case WidgetTypeID::TextBox:
            connectSlotToTextBox(widget, callback, action, async);
        break;
        default:
            std::println("Warning: {0} has no signals other languages can connect to yet.", id);
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::DisconnectSlot(const char *id, CallbackType callback, const i32 action)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch (widgetTypeID)
    {
        case WidgetTypeID::Button:
            disconnectSlotFromButton(widget, callback, action);
        break;
        case WidgetTypeID::Label:
            disconnectSlotFromLabel(widget, callback, action);
        break;
        case WidgetTypeID::TextBox:
            disconnectSlotFromTextBox(widget, callback, action);
        break;
        default:
            [[unlikely]];
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetRotation(const char *id, const glm::vec3& rotation)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return;

    widget->SetRotation(rotation);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetPosition(const char *id, const glm::vec3& position)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return;

    widget->SetPosition(position);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetSize(const char *id, const f32 width, const f32 height)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return;

    widget->SetSize(glm::vec3(width, height, widget->GetSize().z));
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetEnabled(const char* id, const bool enable, const bool emitSignal)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return;

    widget->SetEnabled(enable, emitSignal);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundColors(const char* id, std::span<const glm::vec4> colors, const ColorState colorState)
{
    Background* background { findBackground(id) };

    if(!background)
        return;

    background->SetColors(colors, colorState);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundFillType(const char* id, const FillType fillType)
{
    Background* background { findBackground(id) };

    if(!background)
        return;

    background->SetFillType(fillType);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientOffset(const char *id, const f32 gradientOffset)
{
    Background* background { findBackground(id) };

    if(!background)
        return;

    background->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientAnimationSpeed(const char *id, const f32 animationSpeed)
{
    Background* background { findBackground(id) };

    if(!background)
        return;

    background->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientDegree(const char *id, const f32 degree)
{
    Background* background { findBackground(id) };

    if(!background)
        return;

    background->SetGradientDegree(degree);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientRotationSpeed(const char *id, const f32 rotationSpeed)
{
    Background* background { findBackground(id) };

    if(!background)
        return;

    background->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetWindowBackgroundImageTextureID(const char *id, const u32 textureID)
{
    Background* background { findBackground(id) };

    if(!background)
        return;

    background->SetWindowImageTextureID(textureID);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderColors(const char* id, std::span<const glm::vec4> colors, const ColorState colorState)
{
    Border* border { findBorder(id) };

    if(!border)
        return;

    border->SetColors(colors, colorState);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderFillType(const char* id, const FillType fillType)
{
    Border* border { findBorder(id) };

    if(!border)
        return;

    border->SetFillType(fillType);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientOffset(const char *id, const f32 gradientOffset)
{
    Border* border { findBorder(id) };

    if(!border)
        return;

    border->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientAnimationSpeed(const char *id, const f32 animationSpeed)
{
    Border* border { findBorder(id) };

    if(!border)
        return;

    border->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientDegree(const char *id, const f32 degree)
{
    Border* border { findBorder(id) };

    if(!border)
        return;

    border->SetGradientDegree(degree);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientRotationSpeed(const char *id, const f32 rotationSpeed)
{
    Border* border { findBorder(id) };

    if(!border)
        return;

    border->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetWindowBorderImageTextureID(const char *id, const u32 textureID)
{
    Border* border { findBorder(id) };

    if(!border)
        return;

    border->SetWindowImageTextureID(textureID);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetTextColors(const char *id, std::span<const glm::vec4> colors, const ColorState colorState)
{
    WidgetText* text { findText(id) };

    if(!text)
        return;

    text->SetColors(colors, colorState);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetTextFillType(const char* id, const FillType fillType)
{
    WidgetText* text { findText(id) };

    if(!text)
        return;

    text->SetFillType(fillType);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetText(const char* id, const char* text, const bool emitSignal)
{
    if(!text)
        return;

    IWidget* widget { findWidget(id) };

    if(!widget)
        return;

    TextBox* textBox { dynamic_cast<TextBox*>(widget) };

    //Any other widget shows it as its main text
    if(!textBox)
    {
        if(!widget->_styledText)
            return;

        widget->_styledText->SetString(text, emitSignal);
        return;
    }

    textBox->SetValue(std::string_view { text }, emitSignal); //a TextBox's value is what its text shows, kept in its value store.
}

bool RetroFuturaGUI::DynamicLibWidgetManager::OpenFileExplorer(Window* window, std::string_view title, const i32 width, const i32 height, std::string_view startPath, const u32 flags, std::string_view extensions, PathsCallbackType onResult, CallbackType onDismiss)
{
    if(!window)
        return false;

    std::unique_ptr<FileExplorer> explorer { std::make_unique<FileExplorer>(title, width, height) };

    if(!explorer)
        return false;

    //C strings into the slot's own vector, so they live until onResult returns
    explorer->Connect_OnResult([onResult](std::vector<std::string> paths)
        {
            if(!onResult)
                return;

            std::vector<const char*> pathPointers {};
            pathPointers.reserve(paths.size());

            for(const std::string& path : paths)
                pathPointers.push_back(path.c_str());

            onResult(pathPointers.data(), static_cast<u32>(pathPointers.size()));
        }, false);
    explorer->Connect_OnDismiss([onDismiss]()
        {
            if(!onDismiss)
                return;

            onDismiss();
        }, false);

    if(!explorer->Open(startPath, static_cast<FileExplorer::FileExplorerFlags>(flags), extensions))
        return false; //never pushed, the explorer brings back the context that was current before it

    return window->PushPopup(std::move(explorer));
}

RetroFuturaGUI::DynamicLibWidgetManager& RetroFuturaGUI::DynamicLibWidgetManager::GetInstance()
{
    static DynamicLibWidgetManager Instance;
    return Instance;
}

RetroFuturaGUI::IWidget* RetroFuturaGUI::DynamicLibWidgetManager::findWidget(const char* id)
{
    if(!id)
        return nullptr;

    IWidget* widget { WidgetRegistry::Find(std::string_view { id }) };

    if(!widget)
        std::println("Warning: no widget at {0}. Widgets are named by their full path: \"MainWindow/MainScene/RootLasagna/TestButton\".", id);

    return widget;
}

RetroFuturaGUI::Background* RetroFuturaGUI::DynamicLibWidgetManager::findBackground(const char* id)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return nullptr;

    return dynamic_cast<Background*>(widget->_styledBackground);
}

RetroFuturaGUI::Border* RetroFuturaGUI::DynamicLibWidgetManager::findBorder(const char* id)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return nullptr;

    return dynamic_cast<Border*>(widget->_styledBorder);
}

RetroFuturaGUI::WidgetText* RetroFuturaGUI::DynamicLibWidgetManager::findText(const char* id)
{
    IWidget* widget { findWidget(id) };

    if(!widget)
        return nullptr;

    return widget->_styledText;
}

void RetroFuturaGUI::DynamicLibWidgetManager::connectSlotToButton(IWidget *widget, CallbackType callback, const i32 action, const bool async)
{
    Button* button { dynamic_cast<Button*>(widget) };

    if(!button)
        return;

    switch(action)
    {
        case WidgetAction::OnClick:
            button->Connect_OnClick(callback, async);
        break;
        case WidgetAction::OnRelease:
            button->Connect_OnRelease(callback, async);
        break;
        case WidgetAction::OnMouseEnter:
            button->Connect_OnMouseEnter(callback, async);
        break;
        case WidgetAction::OnMouseLeave:
            button->Connect_OnMouseLeave(callback, async);
        break;
        case WidgetAction::WhileHover:
            button->Connect_WhileHover(callback, async);
        break;
        default:
            std::println("Warning: Action type of {0} is not compatible with Buttons.", action);
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::disconnectSlotFromButton(IWidget *widget, CallbackType callback, const i32 action)
{
    Button* button { dynamic_cast<Button*>(widget) };

    if(!button)
        return;

    switch(action)
    {
        case WidgetAction::OnClick:
            button->Disconnect_OnClick(callback);
        break;
        case WidgetAction::OnRelease:
            button->Disconnect_OnRelease(callback);
        break;
        case WidgetAction::OnMouseEnter:
            button->Disconnect_OnMouseEnter(callback);
        break;
        case WidgetAction::OnMouseLeave:
            button->Disconnect_OnMouseLeave(callback);
        break;
        case WidgetAction::WhileHover:
            button->Disconnect_WhileHover(callback);
        break;
        default:
            std::println("Warning: Action type of {0} is not compatible with Buttons.", action);
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::connectSlotToLabel(IWidget *widget, CallbackType callback, const i32 action, const bool async)
{
    Label* label { dynamic_cast<Label*>(widget) };

    if(!label)
        return;

    switch(action)
    {
        case WidgetAction::OnTextChange:
            label->GetText().Connect_OnTextChange(callback, async);
        break;
        default:
            std::println("Warning: Action type of {0} is not compatible with Labels.", action);
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::disconnectSlotFromLabel(IWidget *widget, CallbackType callback, const i32 action)
{
    Label* label { dynamic_cast<Label*>(widget) };

    if(!label)
        return;

    switch(action)
    {
        case WidgetAction::OnTextChange:
            label->GetText().Disconnect_OnTextChange(callback);
        break;
        default:
            std::println("Warning: Action type of {0} is not compatible with Labels.", action);
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::connectSlotToTextBox(IWidget *widget, CallbackType callback, const i32 action, const bool async)
{
    TextBox* textBox { dynamic_cast<TextBox*>(widget) };

    if(!textBox)
        return;

    switch(action)
    {
        case WidgetAction::OnClick:
            textBox->Connect_OnClick(callback, async);
        break;
        case WidgetAction::OnRelease:
            textBox->Connect_OnRelease(callback, async);
        break;
        case WidgetAction::OnMouseEnter:
            textBox->Connect_OnMouseEnter(callback, async);
        break;
        case WidgetAction::OnMouseLeave:
            textBox->Connect_OnMouseLeave(callback, async);
        break;
        case WidgetAction::WhileHover:
            textBox->Connect_WhileHover(callback, async);
        break;
        case WidgetAction::OnTextChange:
            textBox->GetText().Connect_OnTextChange(callback, async);
        break;
        case WidgetAction::OnEnterPressed:
            textBox->Connect_OnEnterPressed(callback, async);
        break;
        case WidgetAction::OnEnterReleased:
            textBox->Connect_OnEnterReleased(callback, async);
        break;
        default:
            std::println("Warning: Action type of {0} is not compatible with textBoxes.", action);
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::disconnectSlotFromTextBox(IWidget *widget, CallbackType callback, const i32 action)
{
    TextBox* textBox { dynamic_cast<TextBox*>(widget) };

    if(!textBox)
        return;

    switch(action)
    {
        case WidgetAction::OnClick:
            textBox->Disconnect_OnClick(callback);
        break;
        case WidgetAction::OnRelease:
            textBox->Disconnect_OnRelease(callback);
        break;
        case WidgetAction::OnMouseEnter:
            textBox->Disconnect_OnMouseEnter(callback);
        break;
        case WidgetAction::OnMouseLeave:
            textBox->Disconnect_OnMouseLeave(callback);
        break;
        case WidgetAction::WhileHover:
            textBox->Disconnect_WhileHover(callback);
        break;
        case WidgetAction::OnTextChange:
            textBox->GetText().Disconnect_OnTextChange(callback);
        break;
        case WidgetAction::OnEnterPressed:
            textBox->Disconnect_OnEnterPressed(callback);
        break;
        case WidgetAction::OnEnterReleased:
            textBox->Disconnect_OnEnterReleased(callback);
        break;
        default:
            std::println("Warning: Action type of {0} is not compatible with textBoxes.", action);
    }
}