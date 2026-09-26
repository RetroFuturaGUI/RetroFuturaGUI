#include "Prefab.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "config.hpp"
#include <cstddef>
#include <memory>
#include <print>

RetroFuturaGUI::Prefab::Prefab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const AxisDefinition& axisDefinition)
    : RetroFuturaGUI::IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    //Without this the prefab stays WidgetTypeID::None and the dynamic library dispatch never matches it
    _widgetTypeID = WidgetTypeID::Prefab;

    const bool axisDefinitionIsValid { axisDefinition.IsValid() };

    if(!axisDefinitionIsValid)
        std::println("ERROR::PREFAB::INVALID_AXIS_DEFINITION::{}::falling back to a single cell", name);

    //Named after the prefab so grids stay tellable apart in debug output
    const std::string lasagnaName { std::string(name) + "_Lasagna" };
    _lasagna = std::make_unique<Lasagna>(lasagnaName, projection, this, WidgetTypeID::Prefab, parentWindow,
                                         axisDefinitionIsValid ? axisDefinition : SingleCellAxis);

    //Lasagna sizes itself to the whole projection in its constructor. Adopt that so GetSize()/GetPosition()
    //on the prefab report what it actually draws, until a parent grid places it.
    _size = _lasagna->GetSize();
    _position = _lasagna->GetPosition();
}

void RetroFuturaGUI::Prefab::Draw()
{
    if(_lasagna)
        _lasagna->Draw();
}

void RetroFuturaGUI::Prefab::SetSize(const glm::vec3& size)
{
    //The base call keeps _size in step - without it GetSize() on a prefab stays at its constructed value forever
    IWidget::SetSize(size);

    if(_lasagna)
        _lasagna->SetSize(size);
}

void RetroFuturaGUI::Prefab::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_lasagna)
        _lasagna->SetPosition(position);
}

void RetroFuturaGUI::Prefab::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(_lasagna)
        _lasagna->SetRotation(rotation);
}

void RetroFuturaGUI::Prefab::ShowChildWidget(std::string_view name, const bool show)
{
    RetroFuturaGUI::LasagnaCell* cell = findCell(name);

    if(!cell)
        return;

    cell->_Show = show;
}

bool RetroFuturaGUI::Prefab::IsChildWidgetShown(std::string_view name) const
{
    RetroFuturaGUI::LasagnaCell* cell = findCell(name);

    if(!cell)
        return false;

    return cell->_Show;
}

const RetroFuturaGUI::ChildWidget* RetroFuturaGUI::Prefab::findChild(std::string_view name) const
{
    const auto lookupIterator { _widgetLookup.find(Fnv1aHash(name)) };

    if(lookupIterator == _widgetLookup.end())
    {
        std::println("ERROR::PREFAB::CHILD_WIDGET_NOT_FOUND::{}", name);
        return nullptr;
    }

    const ChildWidget& _child { _children[lookupIterator->second] };

    //Keyed by a 32-bit hash, so without this a collision would hand back a different child silently
    if(_child._Widget->GetName() != name)
    {
        std::println("ERROR::PREFAB::CHILD_WIDGET_HASH_COLLISION::{}::resolves to::{}", name, _child._Widget->GetName());
        return nullptr;
    }

    return &_child;
}

RetroFuturaGUI::LasagnaCell* RetroFuturaGUI::Prefab::findCell(std::string_view name) const
{
    const ChildWidget* _child { findChild(name) };

    if(!_child || !_lasagna)
        return nullptr;

    return _lasagna->GetCell(TrackIndex { ._Row = _child->_Placement._Row,
                                          ._Column = _child->_Placement._Column,
                                          ._Layer = _child->_Placement._Layer });
}