#pragma once
#include <span>
#include "Projection.hpp"
#include "ShaderManager.hpp"

namespace RetroFuturaGUI
{
    class LineFrame
    {
    public:
        LineFrame(const GeometryParams2D& geometry, const glm::vec4& color, const f32 borderThickness = 2.0f);
        ~LineFrame();

        void Draw();
        void Resize(const glm::vec2& size);
        void Move(const glm::vec2& position);
        void Rotate(const float rotation);
        void SetColor(const glm::vec4& color);
        glm::vec4 GetColor() const;

    private:
        f32 _testBorderPoints[3 * 4] = 
        {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f
        };

        u32 _testBorderIndices[2 * 4] = 
        {
            0, 1, 
            1, 2, 
            2, 3, 
            3, 0
        };

        u32 _vao;
        u32 _vbo;
        u32 _ebo;
        //glm::vec2 _roundedCorners[8];
        std::unique_ptr<glm::vec4[]> _colors;
        Projection& _projection;
        //glm::vec4 _cornerRadii;
        f32 _borderThickness = 2.0f;

        glm::mat4 _scalingMatrix = glm::mat4(1.0f);
        glm::mat4 _translationMatrix = glm::mat4(0.0f);
        glm::mat4 _rotationMatrix = glm::mat4(1.0f);
        glm::vec2 _scale = glm::vec2(1.0f, 1.0f);
        glm::vec2 _position = glm::vec2(0.0f, 0.0f);
        f32 _rotation = 0.0f;
        u32 _colorCount = 0;
        FillType _fillType = FillType::SOLID;
        void setupMesh();
        void initBasic(std::span<const glm::vec4> colors);
    };
}