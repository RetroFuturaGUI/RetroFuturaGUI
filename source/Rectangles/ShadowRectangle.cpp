#include "ShadowRectangle.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::ShadowRectangle::ShadowRectangle(Projection* projection, DropShadow* shadow)
    : IRectangle(projection), _shadow(shadow)
{
}

void RetroFuturaGUI::ShadowRectangle::SetCaster(const ShadowCaster& caster)
{
    if(!_shadow)
        return;

    //Scaled around the caster's center, then moved; the quad grows by the blur, half of which fades outside the shape
    const f32 scale { _shadow->getShownScale() };
    _blur = _shadow->getShownBlur();
    _shapeSize = caster._Size * scale;
    _ringWidth = caster._RingWidth * scale;
    SetSize(_shapeSize + glm::vec2(_blur + 2.0f));
    SetPosition(caster._Position + glm::vec3(_shadow->getShownOffset(), -_depthOffset));
    SetCornerRadii(caster._CornerRadii * scale);
    SetCutCorners(caster._CutCorners);
}

void RetroFuturaGUI::ShadowRectangle::Draw()
{
    if(!_shadow)
        return;

    if(!_shadow->IsShown())
        return;

    Shader& shader { ShaderManager::GetShadowShader(_shadow->_fillType) };
    shader.UseProgram();
    uploadTransform(shader);
    _shadow->uploadFill(shader, _shaderFeatureDIP & ShaderFeatures::RoundedCorners);
    shader.SetUniformVec2("uShapeHalfSize", _shapeSize * 0.5f);
    shader.SetUniformFloat("uShadowBlur", _blur);
    shader.SetUniformFloat("uRingWidth", _ringWidth);
    drawQuad();
}