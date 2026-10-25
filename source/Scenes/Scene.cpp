#include "Scene.hpp"
#include "IHierarchyNode.hpp"
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "Window.hpp"
#include <memory>
#include <utility>

RetroFuturaGUI::Scene::Scene(std::string_view name, Window* parentWindow)
    : _name(name), _parentWindow(parentWindow)
{

}

void RetroFuturaGUI::Scene::Draw()
{
    if(!_active)
        return;

    if(!_rootContainer)
        return;

    _onUpdate.Emit();
    _onUpdateAsync.EmitAsync();

    _rootContainer->Draw();
}

void RetroFuturaGUI::Scene::Connect_OnUpdate(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onUpdateAsync.Connect(slot);
    else
        _onUpdate.Connect(slot);
}

void RetroFuturaGUI::Scene::Disconnect_OnUpdate(const typename Signal<>::Slot& slot)
{
    _onUpdate.Disconnect(slot);
    _onUpdateAsync.Disconnect(slot);
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

void RetroFuturaGUI::Scene::SetHiddenInTab(const bool hidden)
{
    if(_hiddenInTab == hidden)
        return;

    _hiddenInTab = hidden;
    requestWindowLayout();
}

bool RetroFuturaGUI::Scene::IsHiddenInTab() const
{
    return _hiddenInTab;
}

RetroFuturaGUI::IWidget* RetroFuturaGUI::Scene::GetRootContainer() const
{
    return _rootContainer.get();
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
    SetRootContainer<Lasagna>(projection, axisdefinition);
}

void RetroFuturaGUI::Scene::SetSize(const glm::vec3& size)
{
    _size = size;

    if(!_rootContainer)
        return;

    _rootContainer->SetSize(_size);
}

const glm::vec3& RetroFuturaGUI::Scene::GetSize() const
{
    return _size;
}

void RetroFuturaGUI::Scene::SetPosition(const glm::vec3& position)
{
    _position = position;

    if(!_rootContainer)
        return;

    _rootContainer->SetPosition(_position);
}

const glm::vec3& RetroFuturaGUI::Scene::GetPosition() const
{
    return _position;
}

void RetroFuturaGUI::Scene::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(!_rootContainer)
        return;

    _rootContainer->SetRotation(_rotation);
}

const glm::vec3& RetroFuturaGUI::Scene::GetRotation() const
{
    return _rotation;
}

void RetroFuturaGUI::Scene::SetReservedEdge(const DockEdge edge, const f32 thickness)
{
    const f32 clampedThickness { 0.0f > thickness ? 0.0f : thickness };

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
    if(!_parentWindow)
        return;

    _parentWindow->UpdateLayout();
}

GLFWwindow* RetroFuturaGUI::Scene::parentGlfwWindow() const
{
    if(!_parentWindow)
        return nullptr;

    return _parentWindow->GetGlfwWindow();
}

void RetroFuturaGUI::Scene::adoptRootContainer(std::unique_ptr<IWidget> container)
{
    _rootContainer = std::move(container);

    if(!_rootContainer)
        return;

    if(_size == glm::vec3(0.0f))
        return;

    _rootContainer->SetPosition(_position);
    _rootContainer->SetSize(_size);
    _rootContainer->SetRotation(_rotation);
}