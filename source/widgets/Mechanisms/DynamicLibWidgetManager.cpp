#include "DynamicLibWidgetManager.hpp"
#include "Button.hpp"
#include "Label.hpp"

void RetroFuturaGUI::DynamicLibWidgetManager::AddWidget(std::string_view id, IWidget *widgetRef)
{
    _metaWidgets.emplace(id, widgetRef);
}

void RetroFuturaGUI::DynamicLibWidgetManager::ConnectSlot(const char *id, CallbackType callback, const i32 action, const bool async)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch (widgetTypeID)
    {
        case WidgetTypeID::Button:
            connectSlotToButton(widget, callback, action, async);
        break;
        /*case WidgetTypeID::Label:
            connectSlotFromLabel(widget, callback, action, async);
        break;*/
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::DisconnectSlot(const char *id, CallbackType callback, const i32 action)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch (widgetTypeID)
    {
        case WidgetTypeID::Button:
            disconnectSlotToButton(widget, callback, action);
        break;
        /*case WidgetTypeID::Label:
            disconnectSlotFromLabel(widget, callback, action);
        break;*/
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetRotation(const char *id, const f32 degree)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    widget->SetRotation(degree);
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetSize(const char *id, const f32 width, const f32 height)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    widget->SetSize(glm::vec2(width, height));
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBackgroundColors(colors, colorState);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientOffset(const char *id, const f32 gradientOffset)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBackgroundGradientOffset(gradientOffset);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientAnimationSpeed(const char *id, const f32 animationSpeed)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBackgroundGradientAnimationSpeed(animationSpeed);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientDegree(const char *id, const f32 degree)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBackgroundGradientDegree(degree);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBackgroundGradientRotationSpeed(const char *id, const f32 rotationSpeed)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBackgroundGradientRotationSpeed(rotationSpeed);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetWindowBackgroundImageTextureID(const char *id, const u32 textureID)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetWindowBackgroundImageTextureID(textureID);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBorderColors(colors, colorState);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientOffset(const char *id, const f32 gradientOffset)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBorderGradientOffset(gradientOffset);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientAnimationSpeed(const char *id, const f32 animationSpeed)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBorderGradientAnimationSpeed(animationSpeed);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientDegree(const char *id, const f32 degree)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBorderGradientDegree(degree);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetBorderGradientRotationSpeed(const char *id, const f32 rotationSpeed)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetBorderGradientRotationSpeed(rotationSpeed);
        } break;
    }
}

void RetroFuturaGUI::DynamicLibWidgetManager::SetWindowBorderImageTextureID(const char *id, const u32 textureID)
{
    IWidget* widget { getWidgetPointer(id) };

    if(!widget)
        return;

    WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

    switch(widgetTypeID)
    {
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(widget);
            button->SetWindowBorderImageTextureID(textureID);
        } break;
    }
}

RetroFuturaGUI::DynamicLibWidgetManager& RetroFuturaGUI::DynamicLibWidgetManager::GetInstance()
{
    static DynamicLibWidgetManager Instance;
    return Instance;
}

RetroFuturaGUI::IWidget *RetroFuturaGUI::DynamicLibWidgetManager::getWidgetPointer(const char *id)
{
    if(_metaWidgets.find(id) == _metaWidgets.end())
        return nullptr;

    return _metaWidgets[id];
}

void RetroFuturaGUI::DynamicLibWidgetManager::connectSlotToButton(IWidget *widget, CallbackType callback, const i32 action, const bool async)
{
    Button* button = dynamic_cast<Button*>(widget);

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

/*void RetroFuturaGUI::DynamicLibWidgetManager::connectSlotFromLabel(IWidget *widget, CallbackType callback, const i32 action, const bool async)
{
    Label* label = dynamic_cast<Label*>(widget);

    switch(action)
    {
        //ToDo
        default:
            std::println("Warning: Action type of {0} is not compatible with Labels.", action);
    }
}*/

void RetroFuturaGUI::DynamicLibWidgetManager::disconnectSlotToButton(IWidget *widget, CallbackType callback, const i32 action)
{
    Button* button = dynamic_cast<Button*>(widget);

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

/*void RetroFuturaGUI::DynamicLibWidgetManager::disconnectSlotFromLabel(IWidget *widget, CallbackType callback, const i32 action)
{
    Label* label = dynamic_cast<Label*>(widget);

    switch(action)
    {
        //ToDo
        default:
            std::println("Warning: Action type of {0} is not compatible with Labels.", action);
    }
}*/
