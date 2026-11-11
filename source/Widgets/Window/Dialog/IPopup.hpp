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

    /// @brief The kinds of popups the library builds itself, opened through Window::OpenPopup.
    enum class PopupTypes : u32
    {
        Message, // opens a Popup with a message. Can be closed via the OK button or Close button in the menu bar
        Warning, // same as above
        Error, // same as above
        Value, // Variadic input parameters allow for value input to be returned
        File // opens the FileExplorer
    };

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

        /// @brief Asked when the system closes the popup's window, by Alt+F4 or from the taskbar, rather than Close or Dismiss:
        ///        true lets it close, dismissed; false keeps it open. A popup with work to finish asks the user, and closes itself
        ///        once that is done. True unless overridden.
        virtual bool allowsSystemClose();

        /// @brief Asked when Escape is pressed while this is the top popup: true dismisses it. A popup that takes Escape itself,
        ///        to end an edit first for instance, returns false. True unless overridden
        virtual bool dismissesOnEscape() const;

    private:
        std::unique_ptr<Window> _window { nullptr };
        std::list<std::unique_ptr<ISceneHost>> _scenes {};
        GLFWwindow* _openerContext { nullptr };
        u32 _escapePressCountSeen { 0 };
        bool _isCloseRequested { false }; //Close or Dismiss raised the close flag, not the system
        bool _isOnStack { false }; //PushPopup took it; one that never got there brings its opener's context back when it goes

        /// @brief Sends the values the popup closed with, or that it was dismissed. Called by the window holding the stack, once the popup is closing
        virtual void emitClosed() = 0;

        /// @brief Updates the context again that was current when the popup was created.
        void restoreOpenerContext() const;

        /// @brief Makes the keys arrive at the popup's window, and counts the Escape presses so far as seen. Called by the window
        ///        holding the stack when the popup becomes the top one
        void takeKeys();

        /// @brief Dismisses the popup on an Escape pressed since it last looked. Called by the window holding the stack while this
        ///        is the top popup, before its window takes the input
        void followKeys();

        friend class Window; //calls emitClosed, restoreOpenerContext, allowsSystemClose, takeKeys and followKeys, and sets _isOnStack.
    };
}