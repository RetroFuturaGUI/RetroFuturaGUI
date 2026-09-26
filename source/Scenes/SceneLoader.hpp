#pragma once
#include "Scene.hpp"
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
        SceneLoader(const SceneLoader&) = delete;
        SceneLoader(SceneLoader&&) = delete;
        auto operator =(const SceneLoader&) = delete;
        auto operator =(SceneLoader&&) = delete;

        static SceneLoader& GetInstance()
        {
            static SceneLoader Instance;
            return Instance;
        }

        /// @brief Creates an empty scene and registers it.
        /// @return nullptr when the window is null or the name is already taken.
        std::unique_ptr<Scene> CreateScene(std::string_view name, Window* parentWindow);

        /// @brief Marks a scene for closing. It keeps drawing until the next DrainPending(), which makes this safe to call from a widget callback.
        /// @return false when there is no such scene, or it is already marked.
        bool CloseScene(std::string_view name);

        /// @brief Marks a scene for closing. See the by-name overload.
        /// @return false when there is no such scene, or it is already marked.
        bool CloseScene(const u32 id);

        /// @brief Removes a scene from its window's draw order and from the registry at once, for
        ///        an owner destroying its scene outside the CloseScene/DrainPending cycle - a
        ///        destructor, or shutdown. Call it before the scene is freed.
        /// @return false when there is no such scene.
        bool UnregisterScene(const u32 id);

        /// @brief Removes every scene CloseScene marked from its window's draw order and from the
        ///        registry. Freeing stays with the scene's owner, by dropping the unique_ptr.
        ///        Call once per frame from the frame loop, after the windows have drawn.
        void DrainPending();

        /// @brief A scene marked for closing is still returned until it is drained.
        Scene* GetScene(std::string_view name) const;

        /// @brief A scene marked for closing is still returned until it is drained.
        Scene* GetScene(const u32 id) const;

        /// @brief The ID a name maps to, whether or not that scene exists.
        static u32 GetSceneID(std::string_view name);

        /// @brief 0.0 = 0%, 1.0 = ready, NaN for an unknown ID. Creation is synchronous, so a
        ///        registered scene reports 1.0.
        f32 SceneLoadingProgress(const u32 id) const;

        /// @brief Includes any scene marked for closing.
        uSize GetSceneCount() const;

    private:
        struct SceneEntry
        {
            Scene* _Scene { nullptr }; //Borrowed - CreateScene's caller owns it
            std::string _Name; //Kept so a hash collision cannot silently hand back the wrong scene
            u32 _ID { InvalidSceneID };
            Window* _Window { nullptr };
            bool _PendingClose { false };
        };

        SceneLoader() = default;
        ~SceneLoader() = default;
        SceneEntry* findEntry(const u32 id);
        const SceneEntry* findEntry(const u32 id) const;

    //Elements
        std::list<SceneEntry> _scenes {};

    //Logic
        bool _draining { false }; //Guards DrainPending against re-entering itself
    };
}
