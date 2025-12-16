#pragma once
#include <span>
#include "Projection.hpp"
#include "ShaderManager.hpp"

namespace RetroFuturaGUI
{
    struct BorderParams
    {
        const glm::vec4& _Color;
        const float _Borderthickness;
    };

    class LineBorder
    {
    public:
        LineBorder(const GeometryParams2D& geometry, const BorderParams& borderParams);
        ~LineBorder();

        void Draw();
        void Resize(const glm::vec2& size);
        void Move(const glm::vec2& position);
        void Rotate(const float rotation);
        void SetColor(const glm::vec4& color);
        glm::vec4 GetColor() const;

    private:
        //Geometry
        f32 _testBorderPoints[3 * 4] = 
        {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        u32 _testBorderIndices[2 * 4] = 
        {
            0, 1, 
            1, 2, 
            2, 3, 
            3, 0
        };

        u32
            _vao { 0 },
            _vbo { 0 },
            _ebo { 0 };
        Projection& _projection;
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
        glm::vec2
            _scale { 1.0f },
            _position { 0.0f };
        f32 _rotation { 0.0f };
        u32 _colorCount { 0 };

        // Fill
        FillType _fillType { FillType::SOLID };
        f32 _borderThickness { 2.0f };
        //glm::vec2 _roundedCorners[8];
        std::unique_ptr<glm::vec4[]> _colors;
        //glm::vec4 _cornerRadii = glm::vec4(10.0f);

        void setupMesh();
        void initBasic(std::span<const glm::vec4> colors);
    };
}