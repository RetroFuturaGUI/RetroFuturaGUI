#include "ColorField.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::ColorField::ColorField(Projection* projection)
    : IRectangle(projection)
{
}

void RetroFuturaGUI::ColorField::Draw()
{
    Shader& shader { ShaderManager::GetColorFieldShader(_shape) };
    shader.UseProgram();
    uploadTransform(shader);
    shader.SetUniformFloat("uBrightness", _brightness);
    drawQuad();
}

void RetroFuturaGUI::ColorField::SetShape(const ColorFieldShape shape)
{
    _shape = shape;
}

void RetroFuturaGUI::ColorField::SetBrightness(const f32 brightness)
{
    _brightness = brightness;
}
