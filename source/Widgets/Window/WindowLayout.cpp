#include "Window.hpp"
#include "Scene.hpp"
#include "IDropDown.hpp"
#include "Tooltip.hpp"
#include <algorithm>

void RetroFuturaGUI::Window::SetBackgroundImage(std::string_view imagePath)
{
    IBackground::SetBackgroundImage(imagePath);
}

i32 RetroFuturaGUI::Window::GetBackgroundImageId() const
{
    if(!_backgroundImage)
        return -1;

    return _backgroundImage->GetID();
}

void RetroFuturaGUI::Window::SetLasagna(Lasagna* lasagna)
{
    _lasagna = lasagna;
    UpdateLayout();
}

void RetroFuturaGUI::Window::AddScene(Scene* scene)
{
    if(!scene)
        return;

    if(std::find(_scenes.begin(), _scenes.end(), scene) != _scenes.end())
        return;

    _scenes.push_back(scene);
    UpdateLayout();
}

void RetroFuturaGUI::Window::RemoveScene(Scene* scene)
{
    if(!scene)
        return;

    const std::vector<Scene*>::iterator found { std::find(_scenes.begin(), _scenes.end(), scene) };

    if(found == _scenes.end())
        return;

    _scenes.erase(found);
    UpdateLayout();
}

void RetroFuturaGUI::Window::UpdateLayout()
{
    ScreenRect client { ._Origin = glm::vec2(0.0f), ._Extent = glm::vec2((f32)_width, (f32)_height) };

    //Docked scenes take their strip in the order they were added, so the first one to claim an edge also owns the corner where two claims meet.
    //A scene hidden in a tab reserves nothing while the window is in one.
    for(Scene* scene : _scenes)
    {
        if(!isSceneShown(scene) || !scene->HasReservedEdge())
            continue;

        const ScreenRect strip { reserveEdge(scene->GetReservedEdge(), scene->GetReservedThickness(), client) };

        scene->SetPosition(glm::vec3(strip.GetCenter(), scene->GetPosition().z));
        scene->SetSize(glm::vec3(strip._Extent, scene->GetSize().z));
    }

    _clientRect = client;

    //Overlay scenes and the root Lasagna share whatever the docked scenes left.
    for(Scene* scene : _scenes)
    {
        if(!scene || scene->HasReservedEdge())
            continue;

        scene->SetPosition(glm::vec3(client.GetCenter(), scene->GetPosition().z));
        scene->SetSize(glm::vec3(client._Extent, scene->GetSize().z));
    }

    if(!_lasagna)
        return;

    _lasagna->SetPosition(glm::vec3(client.GetCenter(), _lasagna->GetPosition().z));
    _lasagna->SetSize(glm::vec3(client._Extent, _lasagna->GetSize().z));
}

const RetroFuturaGUI::ScreenRect& RetroFuturaGUI::Window::GetClientRect() const
{
    return _clientRect;
}

void RetroFuturaGUI::Window::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadii = radii;
}

glm::vec4 RetroFuturaGUI::Window::GetDrawnCornerRadii() const
{
    if(IsInTab())
        return _tabCornerRadii;

    glm::vec4 radii { _cornerRadii };

    if(!_noCornerRadiiWhenDocked)
        return radii;

    const glm::bvec4 squared { getSquaredCorners() };

    for(glm::length_t corner = 0; 4 > corner; ++corner)
    {
        if(squared[corner])
            radii[corner] = 0.0f;
    }

    return radii;
}

RetroFuturaGUI::ScreenRect RetroFuturaGUI::Window::reserveEdge(const DockEdge edge, const f32 thickness, ScreenRect& client)
{
    const bool horizontal { edge == DockEdge::Left || edge == DockEdge::Right };
    const f32 taken { std::clamp(thickness, 0.0f, horizontal ? client._Extent.x : client._Extent.y) };

    ScreenRect strip { ._Origin = client._Origin, ._Extent = client._Extent };

    switch(edge)
    {
    case DockEdge::Top:
        strip._Origin.y = client._Origin.y + client._Extent.y - taken;
        strip._Extent.y = taken;
        client._Extent.y -= taken;
    break;
    case DockEdge::Bottom:
        strip._Extent.y = taken;
        client._Origin.y += taken;
        client._Extent.y -= taken;
    break;
    case DockEdge::Left:
        strip._Extent.x = taken;
        client._Origin.x += taken;
        client._Extent.x -= taken;
    break;
    case DockEdge::Right:
        strip._Origin.x = client._Origin.x + client._Extent.x - taken;
        strip._Extent.x = taken;
        client._Extent.x -= taken;
    break;
    }

    return strip;
}

void RetroFuturaGUI::Window::updateProjection()
{
    if(!updateSize())
        return;

    glViewport(0, 0, _width, _height);
}

bool RetroFuturaGUI::Window::updateSize()
{
    if(!_window)
        return false;

    if(!_projection)
        return false;

    glfwGetWindowSize(_window, &_width, &_height);

    if (_lastSize == glm::ivec2(_width, _height))
        return false;

    _projection->UpdateProjectionMatrix((f32)_width, (f32)_height);
    UpdateLayout();
    _lastSize = { _width, _height };
    _windowSizeChanged = false;

    if(!_background)
        return true;

    if(!_border)
        return true;

    _background->SetSize(glm::vec2((f32)_width, (f32)_height));
    _background->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -1.0f));

    _border->SetSize(glm::vec2((f32)_width, (f32)_height));
    _border->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, 0.0f));
    return true;
}

void RetroFuturaGUI::Window::drawContent()
{
    applyCornerRadii();
    drawBackground();
    drawLasagna();

    for(Scene* scene : _scenes)
    {
        if(!isSceneShown(scene))
            continue;

        //Each scene starts with a fresh depth buffer, so a scene added later always draws over the ones loaded before
        glClear(GL_DEPTH_BUFFER_BIT);
        scene->Draw();
    }

    IDropDown::DrawShown(); //after every scene, so an open list covers the widgets drawn after its box
    drawFrame();
    Tooltip::DrawShown(); //last, so it covers every scene
}

void RetroFuturaGUI::Window::drawFrame()
{
    if(!_border)
        return;

    if(IsInTab())
        return; //it would cut the selected field off from the content

    //Width 0, the default, means no frame
    if(0.0f >= _border->GetBorderWidth())
        return;

    //A fresh depth buffer, so the frame lies over every scene
    glClear(GL_DEPTH_BUFFER_BIT);
    drawBorder();
}

void RetroFuturaGUI::Window::drawLasagna()
{
    if(!_lasagna)
        return;

    _lasagna->Draw();
}

void RetroFuturaGUI::Window::applyCornerRadii()
{
    const glm::vec4 radii { GetDrawnCornerRadii() };

    if(radii == _appliedCornerRadii)
        return;

    _appliedCornerRadii = radii;
    setBackgroundCornerRadii(radii);
    setBorderCornerRadii(radii);
}

bool RetroFuturaGUI::Window::isSceneShown(const Scene* scene) const
{
    if(!scene)
        return false;

    if(!scene->IsActive())
        return false;

    if(!IsInTab())
        return true;

    return scene->IsShownInTab();
}
