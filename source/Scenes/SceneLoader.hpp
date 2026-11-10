#pragma once
#include "Scene.hpp"
#include "ResourceManager.hpp"
#include <chrono>
#include <stop_token>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RetroFuturaGUI
{
    class Window;

    inline constexpr u32 InvalidSceneID { 0 };

    /// @brief What a file listed with SceneLoader::AddAsset is loaded as
    enum class AssetKind : u32
    {
        Texture, //an image, as ResourceManager::GetTexture
        Svg, //an SVG's text, which ResourceManager::CreateSvgTexture makes copies from
        Model //a 3D model with its material textures, as Model::LoadModel
    };

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
        ///        Then uploads what the loading threads prepared, a few milliseconds' worth.
        ///        Call once per frame from the frame loop, after the windows have drawn.
        static void DrainPending();

        /// @brief A scene marked for closing is still returned until it is drained.
        static Scene* GetScene(std::string_view name);

        /// @brief A scene marked for closing is still returned until it is drained.
        static Scene* GetScene(const u32 id);

        /// @brief The ID a name maps to, whether or not that scene exists.
        static u32 GetSceneID(std::string_view name);

        /// @brief How many of the files AddAsset queued for the scene are ready: 0.0 none, 1.0 all, a file that failed
        ///        counting as done. A registered scene with none queued reports 1.0, an ID neither registered nor queued for NaN.
        static f32 SceneLoadingProgress(const u32 id);

        /// @brief Includes any scene marked for closing.
        static uSize GetSceneCount();

        /// @brief Loads a file for a scene ahead of the widgets that will show it, and counts it as the scene's. It is read
        ///        and decoded on loading threads and uploaded by DrainPending a few milliseconds' worth a frame, so a loading
        ///        screen keeps drawing; SceneLoadingProgress tells how far it got. The scene needn't exist yet: queue a
        ///        scene's files by its ID (GetSceneID("Level2")), wait for 1.0, then build it, and its widgets find them
        ///        loaded. A widget asking for a file still in preparation gets it finished at once instead of a second load.
        ///        Assets a scene's widgets load are counted on their own; an asset is freed once the last open scene counting
        ///        it is closed and its widgets are gone. One loaded outside any scene is never freed by a scene
        /// @return false when the file isn't there.
        static bool AddAsset(const u32 id, std::string_view path, const AssetKind kind);

        /// @brief How many files count as the scene's, those its widgets loaded and those added. 0 for an unknown ID.
        static uSize GetAssetCount(const u32 id);

        /// @brief While one lives, everything ResourceManager loads counts as the scene's. A scene opens one around its
        ///        Interact, Update and Draw; a scene's setup code opens one right after CreateScene, so what its widgets load
        ///        while they are built counts too. Scopes nest, and the innermost counts
        class AssetScope
        {
        public:
            explicit AssetScope(const Scene& scene);
            AssetScope(const AssetScope&) = delete;
            AssetScope(AssetScope&&) = delete;
            auto operator =(const AssetScope&) = delete;
            auto operator =(AssetScope&&) = delete;
            ~AssetScope();
        };

    private:
        friend class ResourceManager; //tells which files it loads, and asks for one still in preparation

        using PreparedModel = ResourceManager::PreparedModel;

        /// @brief A file AddAsset queued for a scene.
        struct AssetJob
        {
            u32 _SceneID { InvalidSceneID };
            std::string _Path {};
            AssetKind _Kind { AssetKind::Texture };
        };

        /// @brief How many files were queued for a scene and how many are ready.
        struct LoadProgress
        {
            uSize
                _Total { 0 },
                _Ready { 0 };
        };

        struct PreparedAsset; //a file read and decoded on a loading thread, waiting to be uploaded (SceneLoader.cpp)
        struct LoadingThreads; //the loading threads, their queues and lock (SceneLoader.cpp)

        /// @brief The loading threads, started the first time a file is queued.
        static LoadingThreads& getLoadingThreads();

        /// @brief A loading thread: prepares queued files until asked to stop.
        static void runLoadingThread(std::stop_token stop, LoadingThreads& loading);

        /// @brief Reads and decodes a file without GL, so it may run on any thread.
        static PreparedAsset prepareAsset(const AssetJob& job);

        /// @brief Hands a prepared file to ResourceManager, uploading it, counts it for its scene and as ready.
        static void uploadAsset(PreparedAsset&& prepared);

        /// @brief Uploads prepared files until none are left or the frame's budget is spent.
        static void uploadPrepared();

        /// @brief Finishes a file queued or in preparation now, for a widget asking for it. ResourceManager calls it before
        ///        loading a file itself.
        static void finishPreparing(std::string_view path);

        /// @brief Counts a file as a scene's, or, for a scene not created yet, as one waiting for it.
        static void countAsset(const u32 id, std::string_view path);

        /// @brief Counts a file ResourceManager loaded or handed out again as the innermost scope's scene's, or, outside
        ///        any scope, as one no scene may free.
        static void recordAsset(std::string_view path);

        /// @brief Frees the files of a scene that left, unless an open scene counts them too or one was loaded outside any.
        static void freeUnusedAssets(const std::unordered_map<uSize, std::string>& assets);

        struct SceneEntry
        {
            Scene* _Scene { nullptr }; //Borrowed - CreateScene's caller owns it
            std::string _Name; //Kept so a hash collision cannot silently hand back the wrong scene
            u32 _ID { InvalidSceneID };
            Window* _Window { nullptr };
            bool _PendingClose { false };
            std::function<void()> _ReleaseHook {};
            std::unordered_map<uSize, std::string> _Assets {}; //the files counted as the scene's, by their path's hash
        };

        static SceneEntry* findEntry(const u32 id);
        static SceneEntry* findEntry(const Scene* scene);

    //Elements
        static inline std::list<SceneEntry> _scenes {};
        static inline std::vector<const Scene*> _assetScopes {}; //the scenes of the open AssetScopes, innermost last
        static inline std::unordered_set<uSize> _pinnedAssets {}; //loaded outside any scene, so no scene frees them
        static inline std::unordered_map<u32, std::unordered_map<uSize, std::string>> _assetsAhead {}; //queued for scenes not created yet, by their ID
        static inline std::unordered_map<u32, LoadProgress> _progress {}; //by scene ID

    //Logic
        static inline bool
            _draining { false }, //Guards DrainPending against re-entering itself
            _hasLoadingThreads { false };
        static constexpr std::chrono::milliseconds _uploadBudget { 4 }; //per frame, so a loading screen keeps its frame rate
    };
}