#include "InnerGlowRectangle.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::InnerGlowRectangle::InnerGlowRectangle(Projection* projection, InnerGlow* innerGlow)
    : IRectangle(projection), _innerGlow(innerGlow)
{
}

void RetroFuturaGUI::InnerGlowRectangle::SetCaster(const ShadowCaster& caster)
{
    if(!_innerGlow)
        return;

    //As big as the caster: the glow never leaves its shape
    _width = _innerGlow->getShownWidth();
    _offset = _innerGlow->getShownOffset();
    _shapeSize = caster._Size;
    _ringWidth = caster._RingWidth;
    SetSize(_shapeSize);
    SetPosition(caster._Position);
    SetCornerRadii(caster._CornerRadii);
    SetCutCorners(caster._CutCorners);
}

void RetroFuturaGUI::InnerGlowRectangle::Draw()
{
    if(!_innerGlow)
        return;

    if(!_innerGlow->IsShown())
        return;

    if(0.0f >= _width)
        return;

    Shader& shader { ShaderManager::GetInnerGlowShader(_innerGlow->_fillType) };
    shader.UseProgram();
    uploadTransform(shader);
    _innerGlow->uploadFill(shader, _shaderFeatureDIP & ShaderFeatures::RoundedCorners);
    shader.SetUniformVec2("uShapeHalfSize", _shapeSize * 0.5f);
    shader.SetUniformFloat("uGlowWidth", _width);
    shader.SetUniformFloat("uRingWidth", _ringWidth);
    shader.SetUniformVec2("uInnerOffset", _offset);
    drawQuad();
}