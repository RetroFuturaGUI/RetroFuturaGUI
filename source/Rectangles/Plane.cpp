#include "Plane.hpp"
#include "ShaderManager.hpp"
#include <algorithm>

RetroFuturaGUI::Plane::Plane(Projection* projection)
    : IRectangle(projection)
{
}

void RetroFuturaGUI::Plane::Draw()
{
    if(_colors.empty())
        return;

    Shader& shader { ShaderManager::GetPlaneShader(_fillType) };
    uploadPlane(shader);
    drawQuad();
}

void RetroFuturaGUI::Plane::SetBackgroundGaps(const BackgroundGap& gaps)
{
    _backgroundGapData = glm::vec4(
        gaps._Degree,
        gaps._Offset,
        gaps._Length,
        static_cast<f32>(gaps._Repeat)
    );
}

void RetroFuturaGUI::Plane::uploadPlane(Shader& shader)
{
    shader.UseProgram();
    uploadTransform(shader);
    uploadFill(shader, getFogFeatures());
    shader.SetUniformVec4("uBackgroundGap", _backgroundGapData);

    if(0 != getFogFeatures())
        uploadFog(shader);

    uploadImage(shader, _scale);
}