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

RetroFuturaGUI::IWidget* RetroFuturaGUI::DynamicLibWidgetManager::getWidgetPointer(const char *id)
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
