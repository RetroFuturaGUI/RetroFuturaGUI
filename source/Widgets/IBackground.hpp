#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    class IBackground
    {
    public:
        void SetBackgroundColor(const glm::vec4& color, const ColorState state);
        void SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state);
        void SetBackgroundGradientOffset(const f32 gradientOffset);
        void SetBackgroundGradientAnimationSpeed(const f32 animationSpeed);
        void SetBackgroundGradientDegree(const f32 degree);
        void SetBackgroundGradientRotationSpeed(const f32 rotationSpeed);
        void SetWindowBackgroundImageTextureID(const u32 textureID);
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;
        void SetBackgroundFillType(const FillType fillType);

    protected:
        void setBackgroundColors();
        void drawBackground();
        void setBackgroundCornerRadii(const glm::vec4& radii);

    //Elements
        std::unique_ptr<Rectangle> _background;

    //Style
        std::vector<glm::vec4> 
            _backgroundColorEnabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _backgroundColorDisabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _backgroundColorClicked { glm::vec4(0.6f, 0.6f,0.6f, 1.0f) },
            _backgroundColorHover { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) };
        FillType _backgroundFillType { FillType::SOLID };
        ColorState _backgroundColorState { ColorState::Enabled };

    private:
        void setBackgroundColorElement(std::vector<glm::vec4>& color);
    };
}