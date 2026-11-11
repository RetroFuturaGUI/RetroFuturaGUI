#pragma once
#include "IncludeHelper.hpp"
#include "FontManager.hpp"
#include "ISceneHost.hpp"
#include "Lasagna.hpp"
#include "StylingManager.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace RetroFuturaGUI
{
    class Button;
    class Label;
    class MenuBar;
    class Scene;
    class Window;

    /// @brief What the library's own popups share: the FileExplorer, its prompts and the message popups. Their scene, the cells
    ///        their widgets sit in, their font, their frame and bar, and their built-in look, which flavors override. A popup
    ///        inherits it beside its Popup<...> base
    class DialogKit
    {
    protected:
        /// @brief The one font every text in the view uses, so a font set the app prepared covers all of it
        struct ViewFont
        {
            std::string _Family { "Arial" };
            f32 _Size { FontManager::ToPixels(18.0f, FontSizeUnit::Points) }; //in pixels, as every stored font size
            PlatformBridge::Fonts::Slant _Slant { PlatformBridge::Fonts::Slant::Roman };
            PlatformBridge::Fonts::Weight _Weight { PlatformBridge::Fonts::Weight::Normal };
        };

        /// @brief Where a widget sits in the view's Lasagna: one row tall, a number of columns wide
        struct ViewCell
        {
            uSize
                _Row { 0 },
                _Column { 0 },
                _ColumnSpan { 1 };
            SizingMode _SizingMode { SizingMode::FIXED };
            uSize _Layer { 0 }; //widgets sharing a place sit in layers of one cell
        };

        /// @brief Owns the popup's scene and every widget in it. IPopup::AddScene takes it, so the popup destroys the widgets
        ///        while its GL context is current.
        class ViewHost final : public ISceneHost
        {
        public:
            /// @brief Creates the scene in the window, with lasagnaAxis laying out its root Lasagna.
            ViewHost(std::string_view sceneName, Window* window, const AxisDefinition& lasagnaAxis);
            ViewHost() = delete;
            ViewHost(const ViewHost&) = delete;
            ViewHost(ViewHost&&) = delete;
            auto operator =(const ViewHost&) = delete;
            auto operator =(ViewHost&&) = delete;
            ~ViewHost() override;

            Scene* GetScene() const override;

            /// @brief The scene's root Lasagna, laid out by _viewAxis.
            Lasagna* GetLasagna() const;

            /// @brief Builds a widget in the given cell of a Lasagna, which owns it.
            /// @return The widget, or nullptr, building nothing, when the cell is out of range or taken.
            template<typename T, typename... Args>
            static T* Attach(std::string_view name, Lasagna& lasagna, const ViewCell& cell, Args&&... args)
            {
                //Lasagna counts in u32; the view's Lasagna is far below its range
                const CellPlacement placement
                {
                    ._Row = static_cast<u32>(cell._Row),
                    ._Column = static_cast<u32>(cell._Column),
                    ._Layer = static_cast<u32>(cell._Layer),
                    ._SizingMode = cell._SizingMode,
                    ._RowSpan = 1,
                    ._ColSpan = static_cast<u32>(cell._ColumnSpan),
                    ._LayerSpan = 1
                };

                return lasagna.AddWidget<T>(name, placement, std::forward<Args>(args)...);
            }

        private:
            std::unique_ptr<Scene> _scene { nullptr };
        };

        /// @brief The frame every window of the explorer has, its prompts' too: rounded corners and a thin border
        static void styleWindow(Window& window, std::string_view title);

        /// @brief Builds the bar across a Lasagna's first row: the title, which drags the window, and the close button in cell
        ///        _barCloseIndex, which the caller connects.
        static MenuBar* attachBar(ViewHost& host, Lasagna& lasagna, std::string_view title, const uSize columnCount, const f32 barHeight, const glm::vec2& windowSize, const ViewFont& font);

        static void styleButton(Button& button, std::string_view text, const glm::vec3& size, const ViewFont& font);

        /// @brief A line of text on the window's background, a prompt's message for instance.
        static void styleLabel(Label& label, std::string_view text, const glm::vec3& size, const ViewFont& font);

        /// @brief The key that gives the popups' widgets their built-in look, which flavors override
        static StyleKey styleKey();

        /// @brief A table row's height, enough for the font's ascenders and descenders
        static f32 rowHeight(const ViewFont& font);

        /// @brief The size of a fixed-size widget in a cell of a Lasagna laid out by the axis, a margin smaller than the cell on every side
        static glm::vec3 cellSize(const AxisDefinition& axis, const glm::vec2& windowSize, const ViewCell& cell);

        static std::string toUtf8(const std::filesystem::path& path);

        static constexpr f32 _margin { 6.0f };
        static constexpr uSize _barCloseIndex { 2 }; //the bar's flex cell holding the close button
    };
}