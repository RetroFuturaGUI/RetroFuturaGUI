#pragma once
#include "config.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "ShaderManager.hpp"
#include <span>

namespace RetroFuturaGUI
{
    class Rectangle
    {
    public:
        enum class FillType: u32
        {
            SOLID,
            GRADIENT
        };

        Rectangle();
        Rectangle(const glm::vec4& color);
        Rectangle(std::span<const glm::vec4> colors, const float degree = 45.0f, const float animationSpeed = 0.0003f, const float rotationSpeed = 0.02f);
        ~Rectangle();
        void Draw();
        void UpdateAnimationSpeed(const float speed);
        void UpdateDegree(const float degree);
        void UpdateRotationSpeed(const float speed);

    private:
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

        std::unique_ptr<glm::vec4[]> _colors;
        FillType _fillType = FillType::SOLID;
        u32 _colorCount = 0;
        float _gradientOffset = 0.0f;
        float _animationSpeed = 0.0003f;
        float _degree = 45.0f;
        float _rotationSpeed = 0.02f;
        u32 _vao = 0;
        u32 _vbo = 0;
        u32 _ebo = 0;

        void setupMesh();
        void initBasic(std::span<const glm::vec4> colors);
        void drawWithSolidFill();
        void drawWithGradientFill();
    };
}