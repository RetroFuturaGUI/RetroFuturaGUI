#include "Frame.hpp"
#include "ShaderManager.hpp"
#include <algorithm>

RetroFuturaGUI::Frame::Frame(Projection* projection)
    : IRectangle(projection)
{
}

void RetroFuturaGUI::Frame::Draw()
{
    if(_colors.empty())
        return;

    Shader& shader { ShaderManager::GetFrameShader(_fillType) };
    shader.UseProgram();
    uploadTransform(shader);
    uploadFill(shader);
    shader.SetUniformFloat("uBorderWidth", _borderWidth);
    shader.SetUniformInt("uBorderGapCount", static_cast<i32>(_borderGapCount));

    if(0 < _borderGapCount)
        shader.SetUniformVec4("uBorderGaps", &_borderGapData[0][0], static_cast<u32>(_borderGapCount));

    drawQuad();
}

void RetroFuturaGUI::Frame::SetBorderWidth(const f32 width)
{
    _borderWidth = width;
}

f32 RetroFuturaGUI::Frame::GetBorderWidth() const
{
    return _borderWidth;
}

void RetroFuturaGUI::Frame::SetBorderGaps(std::span<BorderGap> gaps)
{
    _borderGapCount = std::min<uSize>(gaps.size(), _maxBorderGapCount);
    _borderGapData.resize(_borderGapCount);

    for(uSize i = 0; i < _borderGapCount; ++i)
    {
        const BorderGap& gap = gaps[i];
        f32 packedEdge = static_cast<f32>(gap._Edge) + (gap._AnchorFarCorner ? 4.0f : 0.0f);
        _borderGapData[i] = glm::vec4(
            packedEdge,
            gap._Offset,
            gap._Length,
            static_cast<f32>(gap._Repeat)
        );
    }
}
