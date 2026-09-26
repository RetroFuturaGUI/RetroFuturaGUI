#pragma once
#include "Scene.hpp"

namespace RetroFuturaGUI
{
    /// @brief Implemented by whatever owns a Scene and the widgets in it, so a project can keep
    ///        scenes of different types in one container and hand each one to Window::AddScene.
    ///        Destroying the host destroys its scene, which is why the destructor is virtual.
    class ISceneHost
    {
    public:
        virtual ~ISceneHost() = default;

        /// @brief The scene this host owns.
        virtual Scene* GetScene() const = 0;
    };
}
