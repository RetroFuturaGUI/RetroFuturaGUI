#include "SceneLoader.hpp"
#include "Window.hpp"
#include <limits>
#include <print>

u32 RetroFuturaGUI::SceneLoader::GetSceneID(std::string_view name)
{
    return Fnv1aHash(name);
}

RetroFuturaGUI::SceneLoader::SceneEntry* RetroFuturaGUI::SceneLoader::findEntry(const u32 id)
{
    for(SceneEntry& _entry : _scenes)
    {
        if(_entry._ID == id)
            return &_entry;
    }

    return nullptr;
}

const RetroFuturaGUI::SceneLoader::SceneEntry* RetroFuturaGUI::SceneLoader::findEntry(const u32 id) const
{
    for(const SceneEntry& _entry : _scenes)
    {
        if(_entry._ID == id)
            return &_entry;
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
        ._PendingClose = false
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
    SceneEntry* _entry { findEntry(id) };

    if(!_entry)
        return false;

    if(_entry->_PendingClose)
        return false;

    _entry->_PendingClose = true;
    return true;
}

void RetroFuturaGUI::SceneLoader::DrainPending()
{
    if(_draining)
        return;

    _draining = true;

    for(std::list<SceneEntry>::iterator _iterator = _scenes.begin(); _iterator != _scenes.end();)
    {
        if(!_iterator->_PendingClose)
        {
            ++_iterator;
            continue;
        }

        //Detach only, the loader borrows, so the owner's unique_ptr is what frees the scene
        if(_iterator->_Window)
            _iterator->_Window->RemoveScene(_iterator->_Scene);

        _iterator = _scenes.erase(_iterator);
    }

    _draining = false;
}

RetroFuturaGUI::Scene* RetroFuturaGUI::SceneLoader::GetScene(std::string_view name) const
{
    const SceneEntry* _entry { findEntry(GetSceneID(name)) };

    if(!_entry)
        return nullptr;

    //A hash is not an identity, so the stored name decides
    if(_entry->_Name != name)
        return nullptr;

    return _entry->_Scene;
}

RetroFuturaGUI::Scene* RetroFuturaGUI::SceneLoader::GetScene(const u32 id) const
{
    const SceneEntry* _entry { findEntry(id) };

    if(!_entry)
        return nullptr;

    return _entry->_Scene;
}

f32 RetroFuturaGUI::SceneLoader::SceneLoadingProgress(const u32 id) const
{
    if(!findEntry(id))
        return std::numeric_limits<f32>::quiet_NaN();

    //Creation is synchronous, so a scene that exists has finished loading
    return 1.0f;
}

uSize RetroFuturaGUI::SceneLoader::GetSceneCount() const
{
    return _scenes.size();
}
