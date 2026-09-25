#include "Scene.hpp"
#include "IHierarchyNode.hpp"
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "Window.hpp"
#include <memory>

RetroFuturaGUI::Scene::Scene(std::string_view name, Window* parentWindow)
    : _name(name), _parentWindow(parentWindow)
{

}

void RetroFuturaGUI::Scene::Draw()
{
    if(!_active)
        return;

    if(_rootLasagna == nullptr)
        return;

    _rootLasagna->Draw();
}

void RetroFuturaGUI::Scene::SetActive(const bool active)
{
    if(_active == active)
        return;

    _active = active;
    requestWindowLayout();
}

bool RetroFuturaGUI::Scene::IsActive() const
{
    return _active;
}

RetroFuturaGUI::Lasagna* RetroFuturaGUI::Scene::GetRootLasagna() const
{
    return _rootLasagna.get();
}

const std::string& RetroFuturaGUI::Scene::GetName() const
{
    return _name;
}

const RetroFuturaGUI::IHierarchyNode* RetroFuturaGUI::Scene::GetParent() const
{
    return _parentWindow;
}

void RetroFuturaGUI::Scene::SetLasagnaAxis(const AxisDefinition axisdefinition, Projection* projection)
{
    if(projection == nullptr)
        return;

    if(_parentWindow == nullptr)
        return;

    _rootLasagna = std::make_unique<Lasagna>("RootLasagna", projection, this, WidgetTypeID::None, _parentWindow->GetGlfwWindow(), axisdefinition);

    if(_size == glm::vec3(0.0f))
        return;

    _rootLasagna->SetPosition(_position);
    _rootLasagna->SetSize(_size);
    _rootLasagna->SetRotation(_rotation);
}

void RetroFuturaGUI::Scene::SetSize(const glm::vec3& size)
{
    _size = size;

    if(_rootLasagna == nullptr)
        return;

    _rootLasagna->SetSize(_size);
}

const glm::vec3& RetroFuturaGUI::Scene::GetSize() const
{
    return _size;
}

void RetroFuturaGUI::Scene::SetPosition(const glm::vec3& position)
{
    _position = position;

    if(_rootLasagna == nullptr)
        return;

    _rootLasagna->SetPosition(_position);
}

const glm::vec3& RetroFuturaGUI::Scene::GetPosition() const
{
    return _position;
}

void RetroFuturaGUI::Scene::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_rootLasagna == nullptr)
        return;

    _rootLasagna->SetRotation(_rotation);
}

const glm::vec3& RetroFuturaGUI::Scene::GetRotation() const
{
    return _rotation;
}

void RetroFuturaGUI::Scene::SetReservedEdge(const DockEdge edge, const f32 thickness)
{
    const f32 clampedThickness { thickness < 0.0f ? 0.0f : thickness };

    if(_hasReservedEdge && _reservedEdge == edge && _reservedThickness == clampedThickness)
        return;

    _hasReservedEdge = true;
    _reservedEdge = edge;
    _reservedThickness = clampedThickness;

    requestWindowLayout();
}

void RetroFuturaGUI::Scene::ClearReservedEdge()
{
    if(!_hasReservedEdge)
        return;

    _hasReservedEdge = false;
    _reservedThickness = 0.0f;

    requestWindowLayout();
}

bool RetroFuturaGUI::Scene::HasReservedEdge() const
{
    return _hasReservedEdge;
}

RetroFuturaGUI::DockEdge RetroFuturaGUI::Scene::GetReservedEdge() const
{
    return _reservedEdge;
}

f32 RetroFuturaGUI::Scene::GetReservedThickness() const
{
    return _reservedThickness;
}

void RetroFuturaGUI::Scene::requestWindowLayout() const
{
    if(_parentWindow == nullptr)
        return;

    _parentWindow->UpdateLayout();
}
