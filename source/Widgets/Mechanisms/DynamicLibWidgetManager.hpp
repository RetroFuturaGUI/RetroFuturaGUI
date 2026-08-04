#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
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
        OnTextChange,
        OnEnterPressed,
        OnEnterReleased,
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
        static void SetEnabled(const char* id, const bool enable);
        static void SetBackgroundColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState);
        static void SetBackgroundGradientOffset(const char* id, const f32 gradientOffset);
        static void SetBackgroundGradientAnimationSpeed(const char* id, const f32 animationSpeed);
        static void SetBackgroundGradientDegree(const char* id, const f32 degree);
        static void SetBackgroundGradientRotationSpeed(const char* id, const f32 rotationSpeed);
        static void SetWindowBackgroundImageTextureID(const char* id, const u32 textureID);
        static void SetBorderColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState);
        static void SetBorderGradientOffset(const char* id, const f32 gradientOffset);
        static void SetBorderGradientAnimationSpeed(const char* id, const f32 animationSpeed);
        static void SetBorderGradientDegree(const char* id, const f32 degree);
        static void SetBorderGradientRotationSpeed(const char* id, const f32 rotationSpeed);
        static void SetWindowBorderImageTextureID(const char* id, const u32 textureID);
        static void SetTextColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState);
        static void SetText(const char* id, const char* text);

    private:
        DynamicLibWidgetManager() = default;
        ~DynamicLibWidgetManager() = default;
        DynamicLibWidgetManager(const DynamicLibWidgetManager&) = delete;
        DynamicLibWidgetManager(DynamicLibWidgetManager&&) = delete;
        auto operator=(const DynamicLibWidgetManager&) = delete;
        auto operator=(DynamicLibWidgetManager&&) = delete;
        static DynamicLibWidgetManager& GetInstance();

        static inline std::unordered_map<std::string, IWidget*> _metaWidgets;
        static IWidget* getWidgetPointer(const char* id);

        //Slots
        static void connectSlotToButton(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromButton(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToLabel(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromLabel(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToTextBox(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromTextBox(IWidget* widget, CallbackType callback, const i32 action);

        //Geometry
    };
}