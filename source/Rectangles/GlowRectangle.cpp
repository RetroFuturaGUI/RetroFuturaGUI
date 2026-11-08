#include "GlowRectangle.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::GlowRectangle::GlowRectangle(Projection* projection, Glow* glow)
    : IRectangle(projection), _glow(glow)
{
}

void RetroFuturaGUI::GlowRectangle::SetCaster(const ShadowCaster& caster, const GlowEdge edge)
{
    if(!_glow)
        return;

    //Outward the quad grows by the width on every side. Inward the glow stays within the ring, as big as the caster
    _edge = edge;
    _width = _glow->GetWidth();
    _shapeSize = caster._Size;
    _ringWidth = caster._RingWidth;
    SetSize(GlowEdge::Outer == edge ? _shapeSize + glm::vec2(_width * 2.0f + 2.0f) : _shapeSize);
    SetPosition(caster._Position);
    SetCornerRadii(caster._CornerRadii);
    SetCutCorners(caster._CutCorners);
}

void RetroFuturaGUI::GlowRectangle::Draw()
{
    if(!_glow)
        return;

    if(!_glow->IsShown())
        return;

    if(0.0f >= _width)
        return;

    Shader& shader { ShaderManager::GetGlowShader(_glow->_fillType) };
    shader.UseProgram();
    uploadTransform(shader);
    _glow->uploadFill(shader, _shaderFeatureDIP & ShaderFeatures::RoundedCorners);
    shader.SetUniformVec2("uShapeHalfSize", _shapeSize * 0.5f);
    shader.SetUniformFloat("uGlowWidth", _width);
    shader.SetUniformFloat("uRingWidth", _ringWidth);
    shader.SetUniformInt("uGlowInward", GlowEdge::Inner == _edge ? 1 : 0);
    drawQuad();
}