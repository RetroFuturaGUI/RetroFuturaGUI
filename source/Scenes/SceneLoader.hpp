#pragma once
#include "Scene.hpp"
#include <list>
#include <memory>
#include <string>
#include <string_view>

namespace RetroFuturaGUI
{
    class Window;

    /// @brief Returned by CreateScene and GetSceneID when there is no such scene.
    inline constexpr u32 InvalidSceneID { 0 };

    /// @brief Owns every Scene's lifetime and hands out non-owning pointers to them
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

        /// @brief Creates an empty scene owned by the loader. Give it a root Lasagna with
        ///        Scene::SetLasagnaAxis, then pass it to Window::AddScene to have it drawn.
        /// @return The scene's ID - the FNV-1a hash of its name, which unlike a position in the
        ///         loader's list stays valid when other scenes are closed. InvalidSceneID when the
        ///         window is null or the name is already taken.
        u32 CreateScene(std::string_view name, Window* parentWindow);

        /// @brief Marks a scene for destruction. It keeps drawing until the next DrainPending(),
        ///        so this is safe to call from a widget callback - which runs inside Window::Draw,
        ///        where destroying a scene outright would free a widget still on the stack.
        /// @return false when there is no such scene, or it is already marked.
        bool CloseScene(std::string_view name);

        /// @brief Marks a scene for destruction. It keeps drawing until the next DrainPending(),
        ///        so this is safe to call from a widget callback - which runs inside Window::Draw,
        ///        where destroying a scene outright would free a widget still on the stack.
        /// @return false when there is no such scene, or it is already marked.
        bool CloseScene(const u32 id);

        /// @brief Destroys everything CloseScene marked, unregistering each scene from its window
        ///        first. Call once per frame from the frame loop, after the windows have drawn -
        ///        never from inside a widget callback.
        void DrainPending();

        /// @brief The scene with this name. A scene marked for destruction is still returned until it is drained.
        Scene* GetScene(std::string_view name) const;

        /// @brief The scene with this ID. A scene marked for destruction is still returned until it is drained.
        Scene* GetScene(const u32 id) const;

        /// @brief The ID a name maps to, whether or not a scene by that name exists.
        static u32 GetSceneID(std::string_view name);

        /// @brief How far along a scene's loading is: 0.0 = 0%, 1.0 = ready, NaN for an unknown ID. Loading is asynchronous
        f32 SceneLoadingProgress(const u32 id) const;

        /// @brief How many scenes the loader owns, including any marked for destruction.
        uSize GetSceneCount() const;

    private:
        struct SceneEntry
        {
            std::unique_ptr<Scene> _Scene { nullptr }; //Kept so a hash collision cannot silently hand back the wrong scene, the same way Prefab verifies its children's names.
            std::string _Name;
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
        bool _draining { false }; //Guards DrainPending against re-entering itself by way of a destructor.


    };
}
