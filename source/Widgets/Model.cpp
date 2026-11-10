#include "Model.hpp"
#include "ResourceManager.hpp"
#include <glm/gtc/matrix_transform.hpp>

RetroFuturaGUI::Model::Model(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Model;
}

void RetroFuturaGUI::Model::LoadModel(std::string_view path)
{
    _meshes = ResourceManager::GetModel(path);
}

void RetroFuturaGUI::Model::Draw()
{
    if(!_meshes)
        return;

    for(Mesh& mesh : *_meshes)
        mesh.Draw(_projection.GetProjectionMatrix(), _translationMatrix, glm::mat4(1.0f), _scalingMatrix); //turned by the widget's frame.
}

void RetroFuturaGUI::Model::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    _scalingMatrix = glm::scale(glm::mat4(1.0f), size);
}

void RetroFuturaGUI::Model::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _translationMatrix = glm::translate(glm::mat4(1.0f), position);
}
