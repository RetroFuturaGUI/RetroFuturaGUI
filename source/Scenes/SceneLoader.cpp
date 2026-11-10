#include "SceneLoader.hpp"
#include "Window.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <optional>
#include <thread>
#include <limits>
#include <print>
#include <vector>

struct RetroFuturaGUI::SceneLoader::PreparedAsset
{
    AssetJob _Job {};
    std::optional<DecodedImage> _Image {};
    std::optional<std::string> _SvgText {};
    std::optional<PreparedModel> _Model {};
};

struct RetroFuturaGUI::SceneLoader::LoadingThreads
{
    std::mutex _Mutex {};
    std::condition_variable_any
        _Ready {}, //a file was queued
        _Done {}; //a file was prepared
    std::deque<AssetJob> _Queued {};
    std::unordered_set<std::string> _InProgress {}; //paths a thread is preparing
    std::deque<PreparedAsset> _Prepared {};
    std::vector<std::jthread> _Threads {}; //last, so they stop and join while the rest still exists
};

u32 RetroFuturaGUI::SceneLoader::GetSceneID(std::string_view name)
{
    return Fnv1aHash(name);
}

RetroFuturaGUI::SceneLoader::SceneEntry* RetroFuturaGUI::SceneLoader::findEntry(const u32 id)
{
    for(SceneEntry& entry : _scenes)
    {
        if(entry._ID == id)
            return &entry;
    }

    return nullptr;
}

std::unique_ptr<RetroFuturaGUI::Scene> RetroFuturaGUI::SceneLoader::CreateScene(std::string_view name, Window* parentWindow)
{
    if(!parentWindow)
    {
        std::println("ERROR::SCENELOADER::NO_PARENT_WINDOW::{}", name);
        return nullptr;
    }

    const u32 id { GetSceneID(name) };

    if(id == InvalidSceneID)
    {
        std::println("ERROR::SCENELOADER::NAME_HASHES_TO_INVALID_ID::{}", name);
        return nullptr;
    }

    if(findEntry(id))
    {
        std::println("ERROR::SCENELOADER::SCENE_NAME_TAKEN::{}", name);
        return nullptr;
    }

    //The entry keeps its own copy of the name so a hash collision can be detected on lookup
    const std::string sceneName { name };
    std::unique_ptr<Scene> created { std::make_unique<Scene>(sceneName, parentWindow) };

    _scenes.push_back(SceneEntry
    {
        ._Scene = created.get(),
        ._Name = sceneName,
        ._ID = id,
        ._Window = parentWindow,
        ._PendingClose = false,
        ._ReleaseHook = {},
        ._Assets = {}
    });

    //Files queued for it before it existed are its own now
    const auto ahead { _assetsAhead.find(id) };

    if(_assetsAhead.end() != ahead)
    {
        _scenes.back()._Assets = std::move(ahead->second);
        _assetsAhead.erase(ahead);
    }

    //Ownership goes to the caller. the entry above only borrows
    return created;
}

bool RetroFuturaGUI::SceneLoader::CloseScene(std::string_view name)
{
    return CloseScene(GetSceneID(name));
}

bool RetroFuturaGUI::SceneLoader::CloseScene(const u32 id)
{
    SceneEntry* entry { findEntry(id) };

    if(!entry)
        return false;

    if(entry->_PendingClose)
        return false;

    entry->_PendingClose = true;
    return true;
}

bool RetroFuturaGUI::SceneLoader::SetReleaseHook(const u32 id, std::function<void()> hook)
{
    SceneEntry* entry { findEntry(id) };

    if(!entry)
        return false;

    entry->_ReleaseHook = std::move(hook);
    return true;
}

bool RetroFuturaGUI::SceneLoader::UnregisterScene(const u32 id)
{
    for(std::list<SceneEntry>::iterator iterator = _scenes.begin(); iterator != _scenes.end(); ++iterator)
    {
        if(iterator->_ID != id)
            continue;

        if(iterator->_Window)
            iterator->_Window->RemoveScene(iterator->_Scene);

        //Its owner frees the scene right after, and its widgets only borrow what they show
        const std::unordered_map<uSize, std::string> assets { std::move(iterator->_Assets) };
        _scenes.erase(iterator);
        freeUnusedAssets(assets);
        return true;
    }

    return false;
}

void RetroFuturaGUI::SceneLoader::DrainPending()
{
    if(_draining)
        return;

    _draining = true;

    std::vector<std::function<void()>> releases;
    std::vector<std::unordered_map<uSize, std::string>> releasedAssets; //only scenes a hook frees: without one their widgets may still show them

    for(std::list<SceneEntry>::iterator iterator = _scenes.begin(); iterator != _scenes.end();)
    {
        if(!iterator->_PendingClose)
        {
            ++iterator;
            continue;
        }

        if(iterator->_Window)
            iterator->_Window->RemoveScene(iterator->_Scene);

        if(iterator->_ReleaseHook)
        {
            releases.push_back(std::move(iterator->_ReleaseHook));
            releasedAssets.push_back(std::move(iterator->_Assets));
        }

        iterator = _scenes.erase(iterator);
    }

    //Hooks run only once the registry has settled. A hook drops the host that owns the scene,
    //and that host's destructor calls UnregisterScene - which must not walk this list while the
    //loop above still holds an iterator into it.
    for(const std::function<void()>& _release : releases)
        _release();

    //Once their widgets are gone
    for(const std::unordered_map<uSize, std::string>& assets : releasedAssets)
        freeUnusedAssets(assets);

    _draining = false;
    uploadPrepared();
}

RetroFuturaGUI::Scene* RetroFuturaGUI::SceneLoader::GetScene(std::string_view name)
{
    const SceneEntry* entry { findEntry(GetSceneID(name)) };

    if(!entry)
        return nullptr;

    //A hash is not an identity, so the stored name decides
    if(entry->_Name != name)
        return nullptr;

    return entry->_Scene;
}

RetroFuturaGUI::Scene* RetroFuturaGUI::SceneLoader::GetScene(const u32 id)
{
    const SceneEntry* entry { findEntry(id) };

    if(!entry)
        return nullptr;

    return entry->_Scene;
}

f32 RetroFuturaGUI::SceneLoader::SceneLoadingProgress(const u32 id)
{
    const auto progress { _progress.find(id) };

    if(_progress.end() != progress && 0 < progress->second._Total)
        return static_cast<f32>(progress->second._Ready) / static_cast<f32>(progress->second._Total);

    if(!findEntry(id))
        return std::numeric_limits<f32>::quiet_NaN();

    return 1.0f; //nothing queued for it
}

uSize RetroFuturaGUI::SceneLoader::GetSceneCount()
{
    return _scenes.size();
}

bool RetroFuturaGUI::SceneLoader::AddAsset(const u32 id, std::string_view path, const AssetKind kind)
{
    if(!std::filesystem::exists(std::filesystem::path(std::u8string(path.begin(), path.end()))))
    {
        std::println("ERROR::SCENELOADER::ASSET_NOT_FOUND::{}", path);
        return false;
    }

    LoadProgress& progress { _progress[id] };
    ++progress._Total;

    if(ResourceManager::IsLoaded(path)) //counts for this scene too, so another one closing keeps it
    {
        countAsset(id, path);
        const auto model { ResourceManager::_models.find(ResourceManager::hashPath(path)) };

        if(ResourceManager::_models.end() != model)
        {
            for(const std::string& texturePath : model->second->_TexturePaths)
                countAsset(id, texturePath);
        }

        ++progress._Ready;
        return true;
    }

    LoadingThreads& loading { getLoadingThreads() };

    {
        const std::lock_guard lock { loading._Mutex };
        loading._Queued.push_back(AssetJob { ._SceneID = id, ._Path = std::string(path), ._Kind = kind });
    }

    loading._Ready.notify_one();
    return true;
}

uSize RetroFuturaGUI::SceneLoader::GetAssetCount(const u32 id)
{
    const SceneEntry* entry { findEntry(id) };

    if(!entry)
        return 0;

    return entry->_Assets.size();
}

RetroFuturaGUI::SceneLoader::AssetScope::AssetScope(const Scene& scene)
{
    _assetScopes.push_back(&scene);
}

RetroFuturaGUI::SceneLoader::AssetScope::~AssetScope()
{
    _assetScopes.pop_back();
}

void RetroFuturaGUI::SceneLoader::recordAsset(std::string_view path)
{
    const uSize hash { ResourceManager::hashPath(path) };

    if(_assetScopes.empty())
    {
        _pinnedAssets.insert(hash);
        return;
    }

    SceneEntry* entry { findEntry(_assetScopes.back()) };

    if(!entry)
    {
        _pinnedAssets.insert(hash); //a scene the loader doesn't know never closes through it
        return;
    }

    entry->_Assets.try_emplace(hash, path);
}

void RetroFuturaGUI::SceneLoader::freeUnusedAssets(const std::unordered_map<uSize, std::string>& assets)
{
    for(const auto& [hash, path] : assets)
    {
        if(_pinnedAssets.contains(hash))
            continue;

        const bool isWaiting { std::any_of(_assetsAhead.begin(), _assetsAhead.end(), [hash](const auto& ahead) { return ahead.second.contains(hash); }) };

        if(isWaiting)
            continue; //queued for a scene not built yet

        const bool isUsed { std::any_of(_scenes.begin(), _scenes.end(), [hash](const SceneEntry& entry) { return entry._Assets.contains(hash); }) };

        if(isUsed)
            continue;

        ResourceManager::Free(path);
    }
}

RetroFuturaGUI::SceneLoader::SceneEntry* RetroFuturaGUI::SceneLoader::findEntry(const Scene* scene)
{
    for(SceneEntry& entry : _scenes)
    {
        if(entry._Scene == scene)
            return &entry;
    }

    return nullptr;
}

RetroFuturaGUI::SceneLoader::LoadingThreads& RetroFuturaGUI::SceneLoader::getLoadingThreads()
{
    static LoadingThreads loading {};

    if(_hasLoadingThreads)
        return loading;

    //Up to four, leaving a core to the GL thread
    const u32
        cores { std::thread::hardware_concurrency() },
        threadCount { 2 > cores ? 1 : std::min<u32>(cores - 1, 4) };

    for(u32 i = 0; i < threadCount; ++i)
        loading._Threads.emplace_back([](std::stop_token stop) { runLoadingThread(stop, loading); });

    _hasLoadingThreads = true;
    return loading;
}

void RetroFuturaGUI::SceneLoader::runLoadingThread(std::stop_token stop, LoadingThreads& loading)
{
    while(true)
    {
        AssetJob job {};

        {
            std::unique_lock lock { loading._Mutex };

            if(!loading._Ready.wait(lock, stop, [&loading]() { return !loading._Queued.empty(); }))
                return; //asked to stop, at exit

            job = std::move(loading._Queued.front());
            loading._Queued.pop_front();
            loading._InProgress.insert(job._Path);
        }

        PreparedAsset prepared { prepareAsset(job) };

        {
            const std::lock_guard lock { loading._Mutex };
            loading._InProgress.erase(prepared._Job._Path);
            loading._Prepared.push_back(std::move(prepared));
        }

        loading._Done.notify_all();
    }
}

RetroFuturaGUI::SceneLoader::PreparedAsset RetroFuturaGUI::SceneLoader::prepareAsset(const AssetJob& job)
{
    PreparedAsset prepared { ._Job = job, ._Image = std::nullopt, ._SvgText = std::nullopt, ._Model = std::nullopt };
    const std::filesystem::path file { std::u8string(job._Path.begin(), job._Path.end()) };

    switch(job._Kind)
    {
        case AssetKind::Texture:
            prepared._Image = Texture::DecodeFile(file, true);
        break;
        case AssetKind::Svg:
        {
            std::ifstream stream { file, std::ios::binary };

            if(stream)
                prepared._SvgText = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
        } break;
        case AssetKind::Model:
            prepared._Model = ResourceManager::prepareModel(job._Path, true);
        break;
        default:
        break;
    }

    return prepared;
}

void RetroFuturaGUI::SceneLoader::uploadAsset(PreparedAsset&& prepared)
{
    const AssetJob& job { prepared._Job };
    bool isLoaded { false };

    switch(job._Kind)
    {
        case AssetKind::Texture:
        {
            if(!prepared._Image)
                break;

            ResourceManager::adoptTexture(job._Path, std::move(*prepared._Image));
            isLoaded = true;
        } break;
        case AssetKind::Svg:
        {
            if(!prepared._SvgText)
                break;

            ResourceManager::adoptSvgSource(job._Path, std::move(*prepared._SvgText));
            isLoaded = true;
        } break;
        case AssetKind::Model:
        {
            if(!prepared._Model)
                break;

            const ResourceManager::LoadedModel* model { ResourceManager::adoptModel(job._Path, std::move(*prepared._Model)) };

            for(const std::string& texturePath : model->_TexturePaths)
                countAsset(job._SceneID, texturePath);

            isLoaded = true;
        } break;
        default:
        break;
    }

    if(isLoaded)
        countAsset(job._SceneID, job._Path);
    else
        std::println("ERROR::SCENELOADER::ASSET_NOT_LOADED::{}", job._Path);

    ++_progress[job._SceneID]._Ready; //a file that failed is done too, so the progress still reaches 1
}

void RetroFuturaGUI::SceneLoader::uploadPrepared()
{
    if(!_hasLoadingThreads)
        return;

    LoadingThreads& loading { getLoadingThreads() };
    const std::chrono::steady_clock::time_point start { std::chrono::steady_clock::now() };

    do
    {
        std::optional<PreparedAsset> prepared {};

        {
            const std::lock_guard lock { loading._Mutex };

            if(loading._Prepared.empty())
                return;

            prepared = std::move(loading._Prepared.front());
            loading._Prepared.pop_front();
        }

        uploadAsset(std::move(*prepared));
    }
    while(std::chrono::steady_clock::now() - start < _uploadBudget);
}

void RetroFuturaGUI::SceneLoader::finishPreparing(std::string_view path)
{
    if(!_hasLoadingThreads)
        return;

    LoadingThreads& loading { getLoadingThreads() };
    std::optional<PreparedAsset> prepared {};
    std::unique_lock lock { loading._Mutex };
    const auto queued { std::find_if(loading._Queued.begin(), loading._Queued.end(), [path](const AssetJob& job) { return path == job._Path; }) };

    if(loading._Queued.end() != queued)
    {
        //Not started yet: prepared here, the widget waits either way
        const AssetJob job { std::move(*queued) };
        loading._Queued.erase(queued);
        lock.unlock();
        prepared = prepareAsset(job);
    }
    else
    {
        loading._Done.wait(lock, [&loading, path]() { return !loading._InProgress.contains(std::string(path)); });
        const auto ready { std::find_if(loading._Prepared.begin(), loading._Prepared.end(), [path](const PreparedAsset& asset) { return path == asset._Job._Path; }) };

        if(loading._Prepared.end() == ready)
            return; //never queued

        prepared = std::move(*ready);
        loading._Prepared.erase(ready);
        lock.unlock();
    }

    uploadAsset(std::move(*prepared));
}

void RetroFuturaGUI::SceneLoader::countAsset(const u32 id, std::string_view path)
{
    const uSize hash { ResourceManager::hashPath(path) };
    SceneEntry* entry { findEntry(id) };

    if(entry)
    {
        entry->_Assets.try_emplace(hash, path);
        return;
    }

    _assetsAhead[id].try_emplace(hash, path);
}
