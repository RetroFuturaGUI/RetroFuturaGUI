#include "Window.hpp"
#include "Scene.hpp"
#include "IDropDown.hpp"
#include "ContextMenu.hpp"
#include "Tooltip.hpp"
#include <algorithm>

i32 RetroFuturaGUI::Window::GetBackgroundImageId() const
{
    if(!_background)
        return -1;

    const u32 textureID { _background->GetImageTextureID() };
    if(0 == textureID)
        return -1;

    return static_cast<i32>(textureID);
}

RetroFuturaGUI::Background& RetroFuturaGUI::Window::GetBackground()
{
    return *_background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::Window::GetBackground() const
{
    return *_background;
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
    scene->emitLoad();
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
    scene->emitUnload();
}

void RetroFuturaGUI::Window::UpdateLayout()
{
    const Transitions::InstantScope instant { true }; //the window lays its scenes out at once, whatever transitions their widgets have
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
    _lasagna->SetSize(glm::vec3(client._Extent, _projection->GetDepth())); //the depth its layers spread over, flat under perspective.
}

void RetroFuturaGUI::Window::SetPerspective(const f32 viewingDistance)
{
    if(!_projection)
        return;

    _projection->SetPerspective(viewingDistance, std::nullopt);
    UpdateLayout();
}

void RetroFuturaGUI::Window::SetOrthographic()
{
    if(!_projection)
        return;

    _projection->SetOrthographic();
    UpdateLayout();
}

const RetroFuturaGUI::ScreenRect& RetroFuturaGUI::Window::GetClientRect() const
{
    return _clientRect;
}

void RetroFuturaGUI::Window::SetTransition(const TransitionProperty properties, const Transition& transition)
{
    Transitions::Override(_ownTransitions, properties, transition);
    StylingManager::ResolveTransitions(_ownTransitions, _flavorHashes, _transitionSettings);
}

void RetroFuturaGUI::Window::ResetTransition(const TransitionProperty properties)
{
    Transitions::Reset(_ownTransitions, properties);
    StylingManager::ResolveTransitions(_ownTransitions, _flavorHashes, _transitionSettings);
}

RetroFuturaGUI::Transition RetroFuturaGUI::Window::GetTransition(const TransitionProperty property) const
{
    return Transitions::Resolve(_transitionSettings, property);
}

glm::vec4 RetroFuturaGUI::Window::GetDrawnCornerRadii() const
{
    return drawnCornerRadii(_backgroundCornerRadii);
}

glm::vec4 RetroFuturaGUI::Window::drawnCornerRadii(const glm::vec4& asked) const
{
    if(IsInTab())
        return _tabCornerRadii;

    glm::vec4 radii { asked };

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
    updateSize();
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
    _background->Draw();
    drawLasagna();

    for(Scene* scene : _scenes)
    {
        if(!isSceneShown(scene))
            continue;

        //Each scene starts with a fresh depth buffer, so a scene added later always draws over the ones loaded before
        Shader::ClearDepth();
        scene->Draw();
    }

    IDropDown::DrawShown(); //after every scene, so an open list covers the widgets drawn after its box
    drawFrame();
    ContextMenu::DrawShown(_window);
    Tooltip::DrawShown(_window); //last, so it covers every scene
}

void RetroFuturaGUI::Window::drawFrame()
{
    if(!_border)
        return;

    if(IsInTab())
        return; //it would cut the selected field off from the content

    //Width 0, the default, means no frame
    if(0.0f >= _border->GetWidth())
        return;

    //A fresh depth buffer, so the frame lies over every scene
    Shader::ClearDepth();
    _border->Draw();
}

void RetroFuturaGUI::Window::drawLasagna()
{
    if(!_lasagna)
        return;

    _lasagna->DrawAtOpacity();
}

void RetroFuturaGUI::Window::updateLasagna()
{
    if(!_lasagna)
        return;

    _lasagna->Update();
}

void RetroFuturaGUI::Window::applyCornerRadii()
{
    const glm::vec4
        backgroundRadii { drawnCornerRadii(_backgroundCornerRadii) },
        borderRadii { drawnCornerRadii(_borderCornerRadii) };

    if(backgroundRadii == _appliedBackgroundCornerRadii && borderRadii == _appliedBorderCornerRadii)
        return;

    _appliedBackgroundCornerRadii = backgroundRadii;
    _appliedBorderCornerRadii = borderRadii;

    if(_background)
        _background->setDrawnCornerRadii(backgroundRadii);

    if(_border)
       _border->setDrawnCornerRadii(borderRadii);
}

void RetroFuturaGUI::Window::takeCornerRadii(glm::vec4& asked, const IWidgetDecoration& part)
{
    asked = part.GetCornerRadii();
    _appliedBackgroundCornerRadii = glm::vec4(-1.0f); //the drawn radii are handed over again at the next draw
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

RetroFuturaGUI::Border& RetroFuturaGUI::Window::GetBorder()
{
    return *_border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::Window::GetBorder() const
{
    return *_border;
}

void RetroFuturaGUI::Window::AddFlavor(std::string_view name)
{
    if(HasFlavor(name))
        return;

    _flavorNames.emplace_back(name);
    StylingManager::composeFlavorHashes(_styleScope, WidgetTypeID::Window, StyleAccessor::None, _flavorNames, _flavorHashes);
    restyleParts();
}

void RetroFuturaGUI::Window::RemoveFlavor(std::string_view name)
{
    const u32 removedHash { StylingManager::GetFlavorHash(name) };
    const auto found { std::ranges::find_if(_flavorNames, [removedHash](const std::string& flavor) { return StylingManager::GetFlavorHash(flavor) == removedHash; }) };

    if(_flavorNames.end() == found)
        return;

    _flavorNames.erase(found);
    StylingManager::composeFlavorHashes(_styleScope, WidgetTypeID::Window, StyleAccessor::None, _flavorNames, _flavorHashes);
    restyleParts();
}

bool RetroFuturaGUI::Window::HasFlavor(std::string_view name) const
{
    const u32 hash { StylingManager::GetFlavorHash(name) };
    return std::ranges::any_of(_flavorNames, [hash](const std::string& flavor) { return StylingManager::GetFlavorHash(flavor) == hash; });
}

void RetroFuturaGUI::Window::setStyleScope(const StyleScope scope)
{
    if(scope == _styleScope)
        return;

    _styleScope = scope;
    StylingManager::composeFlavorHashes(_styleScope, WidgetTypeID::Window, StyleAccessor::None, _flavorNames, _flavorHashes);
    restyleParts();
}

const std::vector<std::string>& RetroFuturaGUI::Window::GetFlavors() const
{
    return _flavorNames;
}

void RetroFuturaGUI::Window::restyleParts()
{
    _styleGeneration = StylingManager::GetGeneration();
    StylingManager::ResolveTransitions(_ownTransitions, _flavorHashes, _transitionSettings);

    if(_background)
        static_cast<IWidgetDecoration&>(*_background).restyle(); //through the base, which lets the window in.

    if(_border)
        static_cast<IWidgetDecoration&>(*_border).restyle();
}

void RetroFuturaGUI::Window::followFlavors()
{
    if(StylingManager::GetGeneration() == _styleGeneration)
        return;

    _styleGeneration = StylingManager::GetGeneration();
    StylingManager::ResolveTransitions(_ownTransitions, _flavorHashes, _transitionSettings);
}
