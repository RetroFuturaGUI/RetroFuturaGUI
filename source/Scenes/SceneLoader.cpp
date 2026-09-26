#include "SceneLoader.hpp"
#include "Window.hpp"
#include <limits>
#include <print>
#include <vector>

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
    std::unique_ptr<Scene> _created { std::make_unique<Scene>(sceneName, parentWindow) };

    _scenes.push_back(SceneEntry
    {
        ._Scene = _created.get(),
        ._Name = sceneName,
        ._ID = id,
        ._Window = parentWindow,
        ._PendingClose = false,
        ._ReleaseHook = {}
    });

    //Ownership goes to the caller. the entry above only borrows
    return _created;
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

        _scenes.erase(iterator);
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
            releases.push_back(std::move(iterator->_ReleaseHook));

        iterator = _scenes.erase(iterator);
    }

    //Hooks run only once the registry has settled. A hook drops the host that owns the scene,
    //and that host's destructor calls UnregisterScene - which must not walk this list while the
    //loop above still holds an iterator into it.
    for(const std::function<void()>& _release : releases)
        _release();

    _draining = false;
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
    if(!findEntry(id))
        return std::numeric_limits<f32>::quiet_NaN();

    //Creation is synchronous, so a scene that exists has finished loading
    return 1.0f;
}

uSize RetroFuturaGUI::SceneLoader::GetSceneCount()
{
    return _scenes.size();
}
