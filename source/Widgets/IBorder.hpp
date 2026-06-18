#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    class IBorder
    {
    public:
        void SetBorderColor(const glm::vec4& color, const ColorState state);
        void SetBorderColors(std::span<glm::vec4> colors, const ColorState state);
        void SetBorderGradientOffset(const f32 gradientOffset);
        void SetBorderGradientAnimationSpeed(const f32 animationSpeed);
        void SetBorderGradientDegree(const f32 degree);
        void SetBorderGradientRotationSpeed(const f32 rotationSpeed);
        void SetWindowBorderImageTextureID(const u32 textureID);
        const std::vector<glm::vec4>& GetBorderColor(const ColorState state) const;
        void SetBorderFillType(const FillType fillType);
        void SetBorderWidth(const f32 borderWidth);

    protected:
        void drawBorder();
        void setBorderColors();
        void setBorderCornerRadii(const glm::vec4& radii);

        std::unique_ptr<Rectangle> _border;
        FillType _borderFillType { FillType::SOLID };
        ColorState _borderColorState { ColorState::Enabled };

        std::vector<glm::vec4> 
            _borderColorEnabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _borderColorDisabled { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) },
            _borderColorClicked { glm::vec4(0.3f, 0.3f, 1.3f, 1.0f) },
            _borderColorHover { glm::vec4(0.275f, 0.275f, 0.275f, 1.0f) };

    private:
        void setBorderColorElement(std::vector<glm::vec4>& color);
    };
}