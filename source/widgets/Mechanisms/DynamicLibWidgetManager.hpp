#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <unordered_map>
#include "Button.hpp"
#include <print>

namespace RetroFuturaGUI
{
    enum WidgetAction : i32
    {
        OnClick,
        OnRelease,
        OnMouseEnter,
        OnMouseLeave,
        WhileHover,
        Unknown = -1
    };  

    extern "C"
    {
        typedef void (*CallbackType)();
    }

    class DynamicLibWidgetManager
    {
    public:

        static void AddWidget(std::string_view id, IWidget* widgetRef)
        {
            _metaWidgets.emplace(id, widgetRef);
        }

        static void ConnectSlot(const char* id, CallbackType callback, const i32 action, const bool async)
        {
            //std::println("id: {}", id);

            if(_metaWidgets.find(id) == _metaWidgets.end())
                return;

            auto& widget = _metaWidgets[id];
            WidgetTypeID widgetTypeID = widget->GetWidgetTypeID();

            switch (widgetTypeID)
            {
                case WidgetTypeID::Button:
                    connectSlotToButton(widget, callback, action, async);
                break;
                case WidgetTypeID::Label:
                    connectSlotToLabel(widget, callback, action, async);
                break;
            }
        }

    private:
        DynamicLibWidgetManager() = default;
        ~DynamicLibWidgetManager() = default;
        DynamicLibWidgetManager(const DynamicLibWidgetManager&) = delete;
        DynamicLibWidgetManager(DynamicLibWidgetManager&&) = delete;
        void operator=(const DynamicLibWidgetManager&) = delete;
        void operator=(DynamicLibWidgetManager&&) = delete;
        static DynamicLibWidgetManager& GetInstance()
        {
            static DynamicLibWidgetManager Instance;
            return Instance;
        }

        static inline std::unordered_map<std::string, IWidget*> _metaWidgets;

        static void connectSlotToButton(IWidget* widget, CallbackType callback, const i32 action, const bool async)
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

        static void connectSlotToLabel(IWidget* widget, CallbackType callback, const i32 action, const bool async)
        {
            Button* button = dynamic_cast<Button*>(widget);

            switch(action)
            {
                default:
                    std::println("Warning: Action type of {0} is not compatible with Labels.", action);
            }
        }
    };
}