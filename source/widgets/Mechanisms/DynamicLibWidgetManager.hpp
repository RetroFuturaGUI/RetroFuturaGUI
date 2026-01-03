#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <print>
#include <unordered_map>

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
        static void AddWidget(std::string_view id, IWidget* widgetRef);
        static void ConnectSlot(const char* id, CallbackType callback, const i32 action, const bool async);
        static void DisconnectSlot(const char* id, CallbackType callback, const i32 action);
        static void SetRotation(const char* id, const f32 degree);
        static void SetSize(const char* id, const f32 width, const f32 height);

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
        static IWidget* getWidgetPointer(const char* id);

        //Slots
        static void connectSlotToButton(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        //static void connectSlotFromLabel(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotToButton(IWidget* widget, CallbackType callback, const i32 action);
        //static void disconnectSlotFromLabel(IWidget* widget, CallbackType callback, const i32 action);

        //Geometry
    };
}