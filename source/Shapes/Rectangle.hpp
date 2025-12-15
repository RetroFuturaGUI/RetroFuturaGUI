#pragma once
#include "ShaderManager.hpp"
#include "Projection.hpp"
#include <span>

namespace RetroFuturaGUI
{
    struct BackgroundParams
    {
        const std::span<const glm::vec4>& _Colors;
        const f32 _GradientDegree = 45.0f;
        const f32 _AnimationSpeed = 0.0003f;
        const f32 _GradientRotationSpeed = 0.02f;
    };

    class Rectangle
    {
    public:
        Rectangle(const GeometryParams2D& geometry, const glm::vec4& color);
        Rectangle(const GeometryParams2D& geometry, const BackgroundParams& background);
        ~Rectangle();
        void Draw();
        void UpdateAnimationSpeed(const f32 speed);
        void UpdateGradientDegree(const f32 degree);
        void UpdateGradientRotationSpeed(const f32 speed);
        void Resize(const glm::vec2& size);
        void Move(const glm::vec2& position);
        void Rotate(const f32 rotation);
        void SetColor(const glm::vec4& color)
        {
            _colors[0] = color;
        }
        glm::vec4 GetColor() const;

    private:
        //Geometry
        f32 _vertices[3 * 4] = 
        {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        u32 _indices[3 * 2] = 
        {
            0, 1, 2,
            2, 3, 0
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
            _scale { 0.01f },
            _position { 0.0f };
        f32 _rotation { 0.0f };
        glm::vec4 _vertexPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        // Fill
        std::unique_ptr<glm::vec4[]> _colors;
        FillType _fillType { FillType::SOLID };
        u32 _colorCount { 0 };
        f32
            _gradientOffset { 0.0f },
            _animationSpeed { 0.0003f },
            _gradientDegree { 45.0f },
            _gradientRotationSpeed { 0.02f };

        void setupMesh();
        void initBasic(std::span<const glm::vec4> colors);
        void drawWithSolidFill();
        void drawWithGradientFill();
    };
}