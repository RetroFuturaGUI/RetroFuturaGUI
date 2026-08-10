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

    /// @brief Exposes widgets to consumers of the dynamic library build (DYNLIB_MODE) via string IDs and C-compatible callbacks, since such consumers cannot hold IWidget pointers directly.
    class DynamicLibWidgetManager
    {
    public:
        /// @brief Registers a widget under the given ID, making it addressable by the other methods on this class.
        static void AddWidget(std::string_view id, IWidget* widgetRef);

        /// @brief Connects a C callback to the given widget's signal for the given WidgetAction.
        /// @param async If true, the callback is invoked asynchronously.
        static void ConnectSlot(const char* id, CallbackType callback, const i32 action, const bool async);

        /// @brief Disconnects a previously connected callback from the given widget's signal for the given WidgetAction.
        static void DisconnectSlot(const char* id, CallbackType callback, const i32 action);

        /// @brief Sets the rotation, in degrees, of the widget with the given ID.
        static void SetRotation(const char* id, const f32 degree);

        /// @brief Sets the size of the widget with the given ID.
        static void SetSize(const char* id, const f32 width, const f32 height);

        /// @brief Enables or disables the widget with the given ID.
        static void SetEnabled(const char* id, const bool enable);

        /// @brief Sets the background colors of the widget with the given ID, for the given color state.
        static void SetBackgroundColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState);

        /// @brief Sets the background gradient offset of the widget with the given ID.
        static void SetBackgroundGradientOffset(const char* id, const f32 gradientOffset);

        /// @brief Sets the background gradient animation speed of the widget with the given ID.
        static void SetBackgroundGradientAnimationSpeed(const char* id, const f32 animationSpeed);

        /// @brief Sets the background gradient angle, in degrees, of the widget with the given ID.
        static void SetBackgroundGradientDegree(const char* id, const f32 degree);

        /// @brief Sets the background gradient rotation speed of the widget with the given ID.
        static void SetBackgroundGradientRotationSpeed(const char* id, const f32 rotationSpeed);

        /// @brief Sets the background image texture ID of the widget with the given ID.
        static void SetWindowBackgroundImageTextureID(const char* id, const u32 textureID);

        /// @brief Sets the border colors of the widget with the given ID, for the given color state.
        static void SetBorderColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState);

        /// @brief Sets the border gradient offset of the widget with the given ID.
        static void SetBorderGradientOffset(const char* id, const f32 gradientOffset);

        /// @brief Sets the border gradient animation speed of the widget with the given ID.
        static void SetBorderGradientAnimationSpeed(const char* id, const f32 animationSpeed);

        /// @brief Sets the border gradient angle, in degrees, of the widget with the given ID.
        static void SetBorderGradientDegree(const char* id, const f32 degree);

        /// @brief Sets the border gradient rotation speed of the widget with the given ID.
        static void SetBorderGradientRotationSpeed(const char* id, const f32 rotationSpeed);

        /// @brief Sets the border image texture ID of the widget with the given ID.
        static void SetWindowBorderImageTextureID(const char* id, const u32 textureID);

        /// @brief Sets the text colors of the widget with the given ID, for the given color state.
        static void SetTextColors(const char* id, std::span<glm::vec4> colors, const ColorState colorState);

        /// @brief Sets the text content of the widget with the given ID.
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