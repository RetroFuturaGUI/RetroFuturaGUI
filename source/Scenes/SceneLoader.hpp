#pragma once
#include "Scene.hpp"
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <string_view>

namespace RetroFuturaGUI
{
    class Window;

    inline constexpr u32 InvalidSceneID { 0 };

    /// @brief Creates scenes and keeps a registry of them.
    class SceneLoader
    {
    public:
        SceneLoader() = delete;
        SceneLoader(const SceneLoader&) = delete;
        SceneLoader(SceneLoader&&) = delete;
        auto operator =(const SceneLoader&) = delete;
        auto operator =(SceneLoader&&) = delete;
        ~SceneLoader() = delete;

        /// @brief Creates an empty scene and registers it.
        /// @return nullptr when the window is null or the name is already taken.
        static std::unique_ptr<Scene> CreateScene(std::string_view name, Window* parentWindow);

        /// @brief Marks a scene for closing. It keeps drawing until the next DrainPending(), which makes this safe to call from a widget callback.
        /// @return false when there is no such scene, or it is already marked.
        static bool CloseScene(std::string_view name);

        /// @brief Marks a scene for closing. See the by-name overload.
        /// @return false when there is no such scene, or it is already marked.
        static bool CloseScene(const u32 id);

        /// @brief Sets what runs when this scene is drained, so that closing it also drops
        ///        whatever owns it. Without a hook a closed scene is only detached and lingers.
        ///        The hook runs after the registry has settled, never while it is being walked.
        /// @return false when there is no such scene.
        static bool SetReleaseHook(const u32 id, std::function<void()> hook);

        /// @brief Removes a scene from its window's draw order and from the registry at once, for
        ///        an owner destroying its scene outside the CloseScene/DrainPending cycle - a
        ///        destructor, or shutdown. Call it before the scene is freed.
        /// @return false when there is no such scene.
        static bool UnregisterScene(const u32 id);

        /// @brief Removes every scene CloseScene marked from its window's draw order and from the
        ///        registry. Freeing stays with the scene's owner, by dropping the unique_ptr.
        ///        Call once per frame from the frame loop, after the windows have drawn.
        static void DrainPending();

        /// @brief A scene marked for closing is still returned until it is drained.
        static Scene* GetScene(std::string_view name);

        /// @brief A scene marked for closing is still returned until it is drained.
        static Scene* GetScene(const u32 id);

        /// @brief The ID a name maps to, whether or not that scene exists.
        static u32 GetSceneID(std::string_view name);

        /// @brief 0.0 = 0%, 1.0 = ready, NaN for an unknown ID. Creation is synchronous, so a
        ///        registered scene reports 1.0.
        static f32 SceneLoadingProgress(const u32 id);

        /// @brief Includes any scene marked for closing.
        static uSize GetSceneCount();

    private:
        struct SceneEntry
        {
            Scene* _Scene { nullptr }; //Borrowed - CreateScene's caller owns it
            std::string _Name; //Kept so a hash collision cannot silently hand back the wrong scene
            u32 _ID { InvalidSceneID };
            Window* _Window { nullptr };
            bool _PendingClose { false };
            std::function<void()> _ReleaseHook;
        };

        static SceneEntry* findEntry(const u32 id);

    //Elements
        static inline std::list<SceneEntry> _scenes {};

    //Logic
        static inline bool _draining { false }; //Guards DrainPending against re-entering itself
    };
}
