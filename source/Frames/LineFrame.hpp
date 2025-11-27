#pragma once
#include <span>
#include "Projection.hpp"
#include "ShaderManager.hpp"

namespace RetroFuturaGUI
{
    class LineFrame
    {
    public:
        LineFrame(Projection& projection, const glm::vec4& color, const f32 width, const f32 height, const f32 positionX = 0.0f, const f32 positionY = 0.0f, const f32 borderThickness = 2.0f, const f32 rotation = 0.0f);
        ~LineFrame();

        void Draw();
        void Resize(const f32 width, const f32 height);
        void Move(const f32 x, const f32 y);
        void Rotate(const float rotation);

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
        f32 _borderThickness;

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