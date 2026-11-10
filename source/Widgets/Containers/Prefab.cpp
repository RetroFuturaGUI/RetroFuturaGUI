#include "Prefab.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "config.hpp"
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

    //Named after the prefab so their Lasagnas stay tellable apart in debug output
    const std::string lasagnaName { std::string(name) + "_Lasagna" };
    _lasagna = std::make_unique<Lasagna>(lasagnaName, projection, this, WidgetTypeID::Prefab, parentWindow,
                                         axisDefinitionIsValid ? axisDefinition : SingleCellAxis);

    //Lasagna sizes itself to the whole projection in its constructor. Adopt that so GetSize()/GetPosition()
    //on the prefab report what it actually draws, until a parent Lasagna places it.
    _size = _lasagna->GetSize();
    _position = _lasagna->GetPosition();
}

void RetroFuturaGUI::Prefab::Draw()
{
    if(!_lasagna)
        return;

    _lasagna->DrawAtOpacity();
}

void RetroFuturaGUI::Prefab::interact()
{
    if(!_lasagna)
        return;

    _lasagna->Interact();
}

void RetroFuturaGUI::Prefab::update()
{
    if(!_lasagna)
        return;

    _lasagna->Update();
}

void RetroFuturaGUI::Prefab::applySize(const glm::vec3& size)
{
    if(!_lasagna)
        return;

    //The base call keeps _size in step - without it GetSize() on a prefab stays at its constructed value forever
    IWidget::applySize(size);

    _lasagna->SetSize(size);
}

void RetroFuturaGUI::Prefab::applyPosition(const glm::vec3& position)
{
    if(!_lasagna)
        return;

    IWidget::applyPosition(position);

    _lasagna->SetPosition(position);
}

std::vector<std::string> RetroFuturaGUI::Prefab::SetAxisDefinition(const AxisDefinition& axisDefinition)
{
    if(!_lasagna)
        return {};

    return _lasagna->SetAxisDefinition(axisDefinition);
}

const RetroFuturaGUI::AxisDefinition& RetroFuturaGUI::Prefab::GetAxisDefinition() const
{
    if(!_lasagna)
        return SingleCellAxis;

    return _lasagna->GetAxisDefinition();
}

std::vector<std::string> RetroFuturaGUI::Prefab::SetColumnDefinition(std::vector<f32> columnDefinition)
{
    if(!_lasagna)
        return {};

    return _lasagna->SetColumnDefinition(std::move(columnDefinition));
}

std::vector<std::string> RetroFuturaGUI::Prefab::SetRowDefinition(std::vector<f32> rowDefinition)
{
    if(!_lasagna)
        return {};

    return _lasagna->SetRowDefinition(std::move(rowDefinition));
}

std::vector<std::string> RetroFuturaGUI::Prefab::SetLayerDefinition(std::vector<f32> layerDefinition)
{
    if(!_lasagna)
        return {};

    return _lasagna->SetLayerDefinition(std::move(layerDefinition));
}

void RetroFuturaGUI::Prefab::ShowChildWidget(std::string_view name, const bool show)
{
    if(!_lasagna)
        return;

    _lasagna->ShowWidget(name, show);
}

bool RetroFuturaGUI::Prefab::IsChildWidgetShown(std::string_view name) const
{
    if(!_lasagna)
        return false;

    return _lasagna->IsWidgetShown(name);
}

bool RetroFuturaGUI::Prefab::RemoveChildWidget(std::string_view name)
{
    if(!_lasagna)
        return false;

    return _lasagna->RemoveWidget(name);
}