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
        f32 _vertices[3 * 4] = 
        {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f
        };

        u32 _indices[3 * 2] = 
        {
            0, 1, 2,
            2, 3, 0
        };

        Projection& _projection;
        glm::mat4 _scalingMatrix = glm::mat4(1.0f);
        glm::mat4 _translationMatrix = glm::mat4(1.0f);
        glm::mat4 _rotationMatrix = glm::mat4(1.0f);
        std::unique_ptr<glm::vec4[]> _colors;
        FillType _fillType = FillType::SOLID;
        u32 _colorCount = 0;
        f32 _gradientOffset = 0.0f;
        f32 _animationSpeed = 0.0003f;
        f32 _gradientDegree = 45.0f;
        f32 _gradientRotationSpeed = 0.02f;
        u32 _vao = 0;
        u32 _vbo = 0;
        u32 _ebo = 0;
        glm::vec2 _scale = glm::vec2(0.01f, 0.01f);
        glm::vec2 _position = glm::vec2(0.0f, 0.0f);
        f32 _rotation = 0.0f;
        glm::vec4 _vertexPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        void setupMesh();
        void initBasic(std::span<const glm::vec4> colors);
        void drawWithSolidFill();
        void drawWithGradientFill();
    };
}