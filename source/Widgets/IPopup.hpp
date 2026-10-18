#pragma once
#include "IncludeHelper.hpp"
#include "ISceneHost.hpp"
#include <list>
#include <memory>
#include <string_view>

namespace RetroFuturaGUI
{
    //Declared, not included, because Window.hpp includes this header
    class Window;

    /// @brief A window of its own that opens over another one through Window::PushPopup, and closes before every popup below
    ///        it. It  owns its own window and the scenes in it. Popup<Ts...> adds the values it closes with; this base is what a
    ///        window's popup stack holds, whatever those types are.
    class IPopup
    {
    public:
        /// @brief Creates the popup's window, hidden until it opens, and leaves its GL context current, so the popup's scenes
        ///        can be built in it right after. Also remembers the context that was current before, which PushPopup makes current again.
        IPopup(std::string_view name, const i32 width, const i32 height);
        IPopup() = delete;
        IPopup(const IPopup&) = delete;
        IPopup(IPopup&&) = delete;
        auto operator =(const IPopup&) = delete;
        auto operator =(IPopup&&) = delete;
        /// @brief Makes the popup's context current, so its scenes delete their GL objects there, then destroys them and the window.
        virtual ~IPopup();

        /// @brief The popup's window, to build its scenes in
        Window* GetWindow() const;

        /// @brief Takes ownership of a scene host and adds its scene to the popup's window, after the ones already added.
        void AddScene(std::unique_ptr<ISceneHost> host);

        /// @brief Closes the popup without values, once the frame is drawn. The window bar's close button does the same.
        /// @return false while another popup lies over this one
        virtual bool Dismiss() = 0;

    protected:
        /// @brief Raises the window's close flag. The window holding the stack acts on it after drawing its popups, once none of this popup's widgets is still running.
        /// @return false while another popup lies over this one.
        bool requestClose();

    private:
        std::unique_ptr<Window> _window { nullptr };
        std::list<std::unique_ptr<ISceneHost>> _scenes {};
        GLFWwindow* _openerContext { nullptr };

        /// @brief Sends the values the popup closed with, or that it was dismissed. Called by the window holding the stack, once the popup is closing
        virtual void emitClosed() = 0;

        /// @brief Updates the context again that was current when the popup was created.
        void restoreOpenerContext() const;

        friend class Window; //calls emitClosed and restoreOpenerContext
    };
}