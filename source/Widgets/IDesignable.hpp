#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    class IDesignable
    {
    public:
    //Background
        void SetBackgroundColor(const glm::vec4& color, const ColorState state);
        void SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state);
        void SetBackgroundGradientOffset(const f32 gradientOffset);
        void SetBackgroundGradientAnimationSpeed(const f32 animationSpeed);
        void SetBackgroundGradientDegree(const f32 degree);
        void SetBackgroundGradientRotationSpeed(const f32 rotationSpeed);
        void SetWindowBackgroundImageTextureID(const u32 textureID);
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;
        void SetBackgroundFillType(const FillType fillType);

    //Border
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

    //Border and Background
        void SetCornerRadii(const glm::vec4& radii);

    protected:
        void setborderBackgroundColors();
        void drawBackgroundBorder();

    //Elements
        std::unique_ptr<Rectangle> _background;
        std::unique_ptr<Rectangle> _border;

    //Style
        std::vector<glm::vec4> 
            _backgroundColorEnabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _backgroundColorDisabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _backgroundColorClicked { glm::vec4(0.6f, 0.6f,0.6f, 1.0f) },
            _backgroundColorHover { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) },
            _borderColorEnabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _borderColorDisabled { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) },
            _borderColorClicked { glm::vec4(0.3f, 0.3f, 1.3f, 1.0f) },
            _borderColorHover { glm::vec4(0.275f, 0.275f, 0.275f, 1.0f) };
        FillType
            _backgroundFillType { FillType::SOLID },
            _borderFillType { FillType::SOLID };
        ColorState _borderBackgroundColorState { ColorState::Enabled };

    private:
        void setBackgroundColorElement(std::vector<glm::vec4>& color);
        void setBorderColorElement(std::vector<glm::vec4>& color);
    };
}