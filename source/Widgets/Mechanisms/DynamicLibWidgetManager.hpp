#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"

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

        /// @brief Takes paths in UTF-8. They are valid during the call only, so a binding copies them before it returns.
        typedef void (*PathsCallbackType)(const char* const* paths, u32 pathCount);
    }

    /// @brief Exposes widgets to consumers of the dynamic library build (DYNLIB_MODE) via string IDs and C-compatible callbacks, since such consumers cannot hold IWidget pointers directly.
    ///        An ID is the widget's full path, as WidgetRegistry finds it. "MainWindow/MainScene/RootLasagna/TestButton". Every
    ///        call with a path no widget has does nothing, so a widget freed meanwhile is never reached.
    ///        Colors, gradients, images and text reach the widget's main background, border and text, the ones its flavors
    ///        style, so every widget that has them takes them; signals reach Buttons, Labels and TextBoxes
    class DynamicLibWidgetManager
    {
    public:
        /// @brief Connects a C callback to the given widget's signal for the given WidgetAction.
        /// @param async If true, the callback is invoked asynchronously.
        static void ConnectSlot(const char* id, CallbackType callback, const i32 action, const bool async);

        /// @brief Disconnects a previously connected callback from the given widget's signal for the given WidgetAction.
        static void DisconnectSlot(const char* id, CallbackType callback, const i32 action);

        /// @brief Sets the rotation, in degrees around x, y and z, of the widget with the given ID.
        static void SetRotation(const char* id, const glm::vec3& rotation);

        /// @brief Sets the position of the widget with the given ID: its center in its window's pixels, and its depth
        static void SetPosition(const char* id, const glm::vec3& position);

        /// @brief Sets the size of the widget with the given ID, keeping its depth
        static void SetSize(const char* id, const f32 width, const f32 height);

        /// @brief Enables or disables the widget with the given ID.
        static void SetEnabled(const char* id, const bool enable, const bool emitSignal);

        /// @brief Sets the background colors of the widget with the given ID, for the given color state.
        static void SetBackgroundColors(const char* id, std::span<const glm::vec4> colors, const ColorState colorState);

        /// @brief Sets how the widget's background spreads its colors: one color (Solid) or a gradient across them
        static void SetBackgroundFillType(const char* id, const FillType fillType);

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
        static void SetBorderColors(const char* id, std::span<const glm::vec4> colors, const ColorState colorState);

        /// @brief Sets how the widget's border spreads its colors: one color (Solid) or a gradient across them
        static void SetBorderFillType(const char* id, const FillType fillType);

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
        static void SetTextColors(const char* id, std::span<const glm::vec4> colors, const ColorState colorState);

        /// @brief Sets how the widget's main text spreads its colors: one color (Solid) or a gradient across them
        static void SetTextFillType(const char* id, const FillType fillType);

        /// @brief Sets the text content of the widget with the given ID: a TextBox's value, any other widget's main text
        /// @param emitSignal Whether the widget's text change signal fires
        static void SetText(const char* id, const char* text, const bool emitSignal);

        /// @brief Opens a FileExplorer over the window, as FileExplorer::Open does with the start path, flags and extensions.
        ///        onResult gets the chosen paths, onDismiss hears it closed without; either may be null.
        /// @param flags FileExplorer::FileExplorerFlags as their bits.
        /// @return false when there is no window, Open refuses the flags, or the window can't take the popup.
        static bool OpenFileExplorer(Window* window, std::string_view title, const i32 width, const i32 height, std::string_view startPath, const u32 flags, std::string_view extensions, PathsCallbackType onResult, CallbackType onDismiss);

    private:
        DynamicLibWidgetManager() = default;
        ~DynamicLibWidgetManager() = default;
        DynamicLibWidgetManager(const DynamicLibWidgetManager&) = delete;
        DynamicLibWidgetManager(DynamicLibWidgetManager&&) = delete;
        auto operator=(const DynamicLibWidgetManager&) = delete;
        auto operator=(DynamicLibWidgetManager&&) = delete;
        static DynamicLibWidgetManager& GetInstance();

        /// @brief The widget at the path, or nullptr when there is none or the path is null
        static IWidget* findWidget(const char* id);

        /// @brief The widget's main background, border or text, the ones its flavors style; nullptr for a widget without one
        static Background* findBackground(const char* id);
        static Border* findBorder(const char* id);
        static WidgetText* findText(const char* id);

        //Slots
        static void connectSlotToButton(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromButton(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToLabel(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromLabel(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToTextBox(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromTextBox(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToEnvironment(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromEnvironment(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToExtendedComboBox(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromExtendedComboBox(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToMenuBar(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromMenuBar(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToPrefab(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromPrefab(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToScrollView(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromScrollView(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToTab(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromTab(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToCheckBox(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromCheckBox(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToColorPicker(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromColorPicker(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToComboBox(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromComboBox(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToHistogram(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromHistogram(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToLineDiagram(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromLineDiagram(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToModel(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromModel(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToMultilineTextBox(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromMultilineTextBox(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToProgressBar(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromProgressBar(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToRadioButton(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromRadioButton(IWidget* widget, CallbackType callback, const i32 action);
        static void connectSlotToWindowTab(IWidget* widget, CallbackType callback, const i32 action, const bool async);
        static void disconnectSlotFromWindowTab(IWidget* widget, CallbackType callback, const i32 action);
    };
}