#pragma once
#include "ShaderManager.hpp"
#include "Projection.hpp"
#include <span>
#include "Image.hpp"

namespace RetroFuturaGUI
{
    enum ShaderFeatures : u32
    {
        RoundedCorners = 1,
        GlassEffect = 1 << 1,
        GlassEffectWithImage = GlassEffect + (1 << 2)
    };

    enum class RectangleMode : u32
    {
        Plane,
        Border
    };

    class Rectangle
    {
    public:
        Rectangle(Projection* projection);
        ~Rectangle();
        void Draw();
        void SetSize(const glm::vec2& size);
        void SetPosition(const glm::vec3& position);
        void SetRotation(const f32 rotation);
        void SetColors(std::span<glm::vec4> colors);
        void SetCornerRadii(const glm::vec4& radii);
        void SetGradientOffset(const f32 gradientOffset);
        void SetGradientAnimationSpeed(const f32 animationSpeed);
        void SetGradientDegree(const f32 degree);
        void SetGradientRotationSpeed(const f32 rotationSpeed);
        void SetShaderFeatures(const u32 features, const bool reset = true);
        void SetWindowBackgroundImageTextureID(const u32 textureID);
        void SetBorderWidth(const f32 width);
        void SetRectangleMode(const RectangleMode rectanlgeMode);
        void SetFillType(const FillType fillType);
        const glm::vec2 GetSize() const;

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
        glm::vec2 _scale { 0.01f };
        glm::vec3 _position { 0.0f };
        f32 _rotation { 0.0f };
        glm::vec4 _vertexPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        // Fill
        std::span<glm::vec4> _colors; //this considers the color source may change many during runtime
        FillType _fillType { FillType::SOLID };
        i32 _colorCount { 0 };
        f32
            _gradientOffset { 0.0f },
            _gradientAnimationSpeed { 0.0003f },
            _gradientDegree { 45.0f },
            _gradientRotationSpeed { 0.02f };
        glm::vec4 _cornerRadii { 0.0f };
        u32 _shaderFeatureDIP { 0 };
        u32 _windowBackgroundTextureID { 0 };
        f32 _borderWidth { 5.0f };
        RectangleMode _rectangleMode { RectangleMode::Plane };
        
        void setupMesh();
        void initColors(std::span<glm::vec4> colors);
        void drawWithSolidFill();
        void drawRadialGradientFill();
        void drawHueStarGradientFill();
        void drawLinearGradientFill();
        void drawSolidBorder();
        void drawLinearGradientBorder();
        void drawRadialGradientBorder();
        void drawHueStarGradientBorder();
    };
}