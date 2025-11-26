#pragma once
#include "config.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ShaderManager.hpp"
#include "Projection.hpp"
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

        Rectangle(Projection& projection, const f32 width, const f32 height);
        Rectangle(Projection& projection, const glm::vec4& color, const f32 width, const f32 height);
        Rectangle(Projection& projection, std::span<const glm::vec4> colors, const f32 width, const f32 height, const f32 degree = 45.0f, const f32 animationSpeed = 0.0003f, const f32 rotationSpeed = 0.02f);
        ~Rectangle();
        void Draw();
        void UpdateAnimationSpeed(const f32 speed);
        void UpdateDegree(const f32 degree);
        void UpdateRotationSpeed(const f32 speed);
        void Resize(const f32 width, const f32 height);
        void Move(const f32 x, const f32 y);
        void Rotate(const float degree);

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
        f32 _degree = 45.0f;
        f32 _rotationSpeed = 0.02f;
        u32 _vao = 0;
        u32 _vbo = 0;
        u32 _ebo = 0;
        glm::vec2 _position = glm::vec2(0.0f, 0.0f);
        f32 _rotation = 0.0f;
        glm::vec4 _vertexPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        const f32 _ndc[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f} };
        


        void setupMesh();
        void initBasic(std::span<const glm::vec4> colors);
        void drawWithSolidFill();
        void drawWithGradientFill();
    };
}