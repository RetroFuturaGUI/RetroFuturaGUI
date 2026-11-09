#pragma once
#include "IncludeHelper.hpp"
#include <concepts>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace RetroFuturaGUI
{
    class IWidget;

    /// @brief Every widget that exists, found by its path ("MainWindow/MainScene/RootLasagna/OkButton") or by the hash of
    ///        it (IWidget::GetPathHash, Fnv1aHash of the path). Widgets add themselves when they are created and leave when
    ///        they are destroyed. A rename changes the paths of everything below the widget, so the hashes are worked out
    ///        again on the first lookup after widgets came, went or were renamed. Used from the GUI thread only
    RFG_EXPORT class WidgetRegistry final
    {
    public:
        WidgetRegistry() = delete;

        /// @brief Returns the widget at the path, or nullptr when there is none
        static IWidget* Find(std::string_view path);

        /// @brief Returns the widget at the path, or nullptr when there is none or it isn't a T
        template<std::derived_from<IWidget> T> static T* Find(std::string_view path)
        {
            return dynamic_cast<T*>(Find(path));
        }

        /// @brief Returns the widget whose path has this hash, or nullptr when there is none. Two paths can share a hash, if
        ///        rarely; this returns the first widget with it, while Find(path) tells them apart
        static IWidget* Find(const u32 pathHash);

        /// @brief Returns the widget whose path has this hash, or nullptr when there is none or it isn't a T
        template<std::derived_from<IWidget> T> static T* Find(const u32 pathHash)
        {
            return dynamic_cast<T*>(Find(pathHash));
        }

        /// @brief Returns how many widgets exist.
        static uSize GetWidgetCount();

    private:
        friend class IWidget; //adds, removes and renames itself. Asks for its own hash

        static void add(IWidget* widget);
        static void remove(IWidget* widget);

        /// @brief Has the hashes worked out again on the next lookup. A widget came, went or was renamed
        static void markChanged();

        /// @brief Returns the up-to-date widget's path hash
        static u32 getPathHash(const IWidget& widget);

        /// @brief Works out every widget's path hash again when something changed since the last time
        static void refresh();

        static inline std::vector<IWidget*> _widgets {};
        static inline std::unordered_map<u32, std::vector<IWidget*>> _byPathHash {};
        static inline bool _isChanged { false };
    };
}