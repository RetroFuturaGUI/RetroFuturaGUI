#pragma once
#include "IncludeHelper.hpp"
#include "StylingManager.hpp"
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief One setting of a part through the style layers: the part's own value when set, else the first of its widget's
    ///        flavors that sets it, then .default's, else the element's default. Keeps the value last handed to the element,
    ///        which an element showing a list or a span points at, so it stays put until the value changes
    template<typename T> class LayeredSetting final
    {
    public:
        /// @brief Sets the part's own value. Set again, it is handed on again, so setting a gradient's angle restarts it
        void Set(T value)
        {
            _own = std::move(value);
            _isOwnPending = true;
        }

        /// @brief Leaves the setting to the flavors and the default
        void Reset()
        {
            _own.reset();
        }

        /// @brief Returns the value last handed to the element, or nothing while none was
        const std::optional<T>& GetShown() const
        {
            return _shown;
        }

        /// @brief Takes the value through the layers and returns it when it should go to the element, nullptr when not:
        ///        it didn't change, or nothing ever set it, so the element keeps its own default
        /// @param isStyled Whether the part takes its widget's flavors at all: a sub-part goes from its own straight to the default
        template<typename Pick> T* Resolve(std::span<const u32> flavorHashes, const bool isStyled, const StylePart part, const T& builtIn, Pick pick)
        {
            std::optional<T> chosen { _own };

            if(!chosen && isStyled)
                chosen = StylingManager::FindFirst<std::optional<T>>(flavorHashes, part, pick);

            if(!chosen)
            {
                if(!_shown)
                    return nullptr;

                chosen = builtIn; //it was set before: back to the element's default
            }

            if(_shown && *_shown == *chosen && !_isOwnPending)
                return nullptr;

            _isOwnPending = false;
            _shown = std::move(chosen);
            return &*_shown;
        }

    private:
        std::optional<T>
            _own {},
            _shown {};
        bool _isOwnPending { false };
    };

    /// @brief A part's shader settings through the style layers, one LayeredSetting each. Apply hands an element only the
    ///        settings it has and whose value changed, so a flavor changing elsewhere doesn't restart a gradient that is turning.
    ///        Kept by the parts; the members are the settings, named as the parts' setters
    struct ShaderLayers final
    {
        LayeredSetting<FillType> _FillType {};
        LayeredSetting<f32> _GradientDegree {};
        LayeredSetting<f32> _GradientOffset {};
        LayeredSetting<f32> _GradientAnimationSpeed {};
        LayeredSetting<f32> _GradientRotationSpeed {};
        LayeredSetting<RasterPattern> _RasterPattern {};
        LayeredSetting<glm::vec4> _PrimaryRasterColor {};
        LayeredSetting<glm::vec4> _SecondaryRasterColor {};
        LayeredSetting<f32> _RasterSpacing {};
        LayeredSetting<f32> _RasterDegree {};
        LayeredSetting<std::vector<f32>> _PrimaryRasterWidthTransfer {};
        LayeredSetting<f32> _DotTransparencyTransfer {};
        LayeredSetting<f32> _RasterAnimationSpeed {};
        LayeredSetting<u32> _WindowImageTextureID {};
        LayeredSetting<GlassNoise> _GlassNoise {};
        LayeredSetting<f32> _FogAlpha {};
        LayeredSetting<f32> _FogSpeed {};
        LayeredSetting<std::vector<f32>> _FogDensity {};
        LayeredSetting<f32> _FogClearing {};
        LayeredSetting<FogNoise> _FogNoise {};
        LayeredSetting<f32> _ImagePadding {};
        LayeredSetting<ImageFit> _ImageFit {};
        LayeredSetting<u32> _MaskTextureID {};
        LayeredSetting<MaskMode> _MaskMode {};
        LayeredSetting<ImageFit> _MaskFit {};
        LayeredSetting<BackgroundGap> _BackgroundGaps {};
        LayeredSetting<std::vector<BorderGap>> _BorderGaps {};
        LayeredSetting<f32> _PixelateBlockSize {};
        LayeredSetting<CrtEffect> _CrtEffect {};

        /// @brief Hands the element every setting it has whose value through the layers changed since it was last handed over
        template<typename Element> void Apply(Element& element, std::span<const u32> flavorHashes, const bool isStyled, const StylePart part)
        {
            if constexpr (requires(Element& probe, FillType value) { probe.SetFillType(value); })
            {
                if(FillType* v { _FillType.Resolve(flavorHashes, isStyled, part, FillType::Solid, [](const FlavorPart& style) -> const std::optional<FillType>& { return style.GetFillType(); }) })
                    element.SetFillType(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetGradientDegree(value); })
            {
                if(f32* v { _GradientDegree.Resolve(flavorHashes, isStyled, part, 45.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientDegree(); }) })
                    element.SetGradientDegree(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetGradientOffset(value); })
            {
                if(f32* v { _GradientOffset.Resolve(flavorHashes, isStyled, part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientOffset(); }) })
                    element.SetGradientOffset(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetGradientAnimationSpeed(value); })
            {
                if(f32* v { _GradientAnimationSpeed.Resolve(flavorHashes, isStyled, part, 0.018f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientAnimationSpeed(); }) })
                    element.SetGradientAnimationSpeed(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetGradientRotationSpeed(value); })
            {
                if(f32* v { _GradientRotationSpeed.Resolve(flavorHashes, isStyled, part, 1.2f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientRotationSpeed(); }) })
                    element.SetGradientRotationSpeed(*v);
            }

            if constexpr (requires(Element& probe, RasterPattern value) { probe.SetRasterPattern(value); })
            {
                if(RasterPattern* v { _RasterPattern.Resolve(flavorHashes, isStyled, part, RasterPattern::Dotted, [](const FlavorPart& style) -> const std::optional<RasterPattern>& { return style.GetRasterPattern(); }) })
                    element.SetRasterPattern(*v);
            }

            if constexpr (requires(Element& probe, glm::vec4 value) { probe.SetPrimaryRasterColor(value); })
            {
                if(glm::vec4* v { _PrimaryRasterColor.Resolve(flavorHashes, isStyled, part, glm::vec4(1.0f), [](const FlavorPart& style) -> const std::optional<glm::vec4>& { return style.GetPrimaryRasterColor(); }) })
                    element.SetPrimaryRasterColor(*v);
            }

            if constexpr (requires(Element& probe, glm::vec4 value) { probe.SetSecondaryRasterColor(value); })
            {
                if(glm::vec4* v { _SecondaryRasterColor.Resolve(flavorHashes, isStyled, part, glm::vec4(1.0f), [](const FlavorPart& style) -> const std::optional<glm::vec4>& { return style.GetSecondaryRasterColor(); }) })
                    element.SetSecondaryRasterColor(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetRasterSpacing(value); })
            {
                if(f32* v { _RasterSpacing.Resolve(flavorHashes, isStyled, part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetRasterSpacing(); }) })
                    element.SetRasterSpacing(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetRasterDegree(value); })
            {
                if(f32* v { _RasterDegree.Resolve(flavorHashes, isStyled, part, 45.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetRasterDegree(); }) })
                    element.SetRasterDegree(*v);
            }

            if constexpr (requires(Element& probe, std::vector<f32> value) { probe.SetPrimaryRasterWidthTransfer(std::span<f32>(value)); })
            {
                if(std::vector<f32>* v { _PrimaryRasterWidthTransfer.Resolve(flavorHashes, isStyled, part, std::vector<f32> {}, [](const FlavorPart& style) -> const std::optional<std::vector<f32>>& { return style.GetPrimaryRasterWidthTransfer(); }) })
                    element.SetPrimaryRasterWidthTransfer(std::span<f32>(*v));
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetDotTransparencyTransfer(value); })
            {
                if(f32* v { _DotTransparencyTransfer.Resolve(flavorHashes, isStyled, part, 1.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetDotTransparencyTransfer(); }) })
                    element.SetDotTransparencyTransfer(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetRasterAnimationSpeed(value); })
            {
                if(f32* v { _RasterAnimationSpeed.Resolve(flavorHashes, isStyled, part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetRasterAnimationSpeed(); }) })
                    element.SetRasterAnimationSpeed(*v);
            }

            if constexpr (requires(Element& probe, u32 value) { probe.SetWindowBackgroundImageTextureID(value); })
            {
                if(u32* v { _WindowImageTextureID.Resolve(flavorHashes, isStyled, part, 0u, [](const FlavorPart& style) -> const std::optional<u32>& { return style.GetWindowImageTextureID(); }) })
                    element.SetWindowBackgroundImageTextureID(*v);
            }

            if constexpr (requires(Element& probe, GlassNoise value) { probe.SetGlassNoise(value); })
            {
                if(GlassNoise* v { _GlassNoise.Resolve(flavorHashes, isStyled, part, GlassNoise {}, [](const FlavorPart& style) -> const std::optional<GlassNoise>& { return style.GetGlassNoise(); }) })
                    element.SetGlassNoise(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetFogAlpha(value); })
            {
                if(f32* v { _FogAlpha.Resolve(flavorHashes, isStyled, part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetFogAlpha(); }) })
                    element.SetFogAlpha(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetFogSpeed(value); })
            {
                if(f32* v { _FogSpeed.Resolve(flavorHashes, isStyled, part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetFogSpeed(); }) })
                    element.SetFogSpeed(*v);
            }

            if constexpr (requires(Element& probe, std::vector<f32> value) { probe.SetFogDensity(std::span<f32>(value)); })
            {
                if(std::vector<f32>* v { _FogDensity.Resolve(flavorHashes, isStyled, part, std::vector<f32> {}, [](const FlavorPart& style) -> const std::optional<std::vector<f32>>& { return style.GetFogDensity(); }) })
                    element.SetFogDensity(std::span<f32>(*v));
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetFogClearing(value); })
            {
                if(f32* v { _FogClearing.Resolve(flavorHashes, isStyled, part, 0.5f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetFogClearing(); }) })
                    element.SetFogClearing(*v);
            }

            if constexpr (requires(Element& probe, FogNoise value) { probe.SetFogNoise(value); })
            {
                if(FogNoise* v { _FogNoise.Resolve(flavorHashes, isStyled, part, FogNoise {}, [](const FlavorPart& style) -> const std::optional<FogNoise>& { return style.GetFogNoise(); }) })
                    element.SetFogNoise(*v);
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetBackgroundImagePadding(value); })
            {
                if(f32* v { _ImagePadding.Resolve(flavorHashes, isStyled, part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetImagePadding(); }) })
                    element.SetBackgroundImagePadding(*v);
            }

            if constexpr (requires(Element& probe, ImageFit value) { probe.SetBackgroundImageFit(value); })
            {
                if(ImageFit* v { _ImageFit.Resolve(flavorHashes, isStyled, part, ImageFit::Stretch, [](const FlavorPart& style) -> const std::optional<ImageFit>& { return style.GetImageFit(); }) })
                    element.SetBackgroundImageFit(*v);
            }

            if constexpr (requires(Element& probe, u32 value) { probe.SetMaskTextureID(value); })
            {
                if(u32* v { _MaskTextureID.Resolve(flavorHashes, isStyled, part, 0u, [](const FlavorPart& style) -> const std::optional<u32>& { return style.GetMaskTextureID(); }) })
                    element.SetMaskTextureID(*v);
            }

            if constexpr (requires(Element& probe, MaskMode value) { probe.SetMaskMode(value); })
            {
                if(MaskMode* v { _MaskMode.Resolve(flavorHashes, isStyled, part, MaskMode::Alpha, [](const FlavorPart& style) -> const std::optional<MaskMode>& { return style.GetMaskMode(); }) })
                    element.SetMaskMode(*v);
            }

            if constexpr (requires(Element& probe, ImageFit value) { probe.SetMaskFit(value); })
            {
                if(ImageFit* v { _MaskFit.Resolve(flavorHashes, isStyled, part, ImageFit::Stretch, [](const FlavorPart& style) -> const std::optional<ImageFit>& { return style.GetMaskFit(); }) })
                    element.SetMaskFit(*v);
            }

            if constexpr (requires(Element& probe, BackgroundGap value) { probe.SetBackgroundGaps(value); })
            {
                if(BackgroundGap* v { _BackgroundGaps.Resolve(flavorHashes, isStyled, part, BackgroundGap {}, [](const FlavorPart& style) -> const std::optional<BackgroundGap>& { return style.GetBackgroundGaps(); }) })
                    element.SetBackgroundGaps(*v);
            }

            if constexpr (requires(Element& probe, std::vector<BorderGap> value) { probe.SetBorderGaps(std::span<BorderGap>(value)); })
            {
                if(std::vector<BorderGap>* v { _BorderGaps.Resolve(flavorHashes, isStyled, part, std::vector<BorderGap> {}, [](const FlavorPart& style) -> const std::optional<std::vector<BorderGap>>& { return style.GetBorderGaps(); }) })
                    element.SetBorderGaps(std::span<BorderGap>(*v));
            }

            if constexpr (requires(Element& probe, f32 value) { probe.SetPixelateBlockSize(value); })
            {
                if(f32* v { _PixelateBlockSize.Resolve(flavorHashes, isStyled, part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetPixelateBlockSize(); }) })
                    element.SetPixelateBlockSize(*v);
            }

            if constexpr (requires(Element& probe, CrtEffect value) { probe.SetCrtEffect(value); })
            {
                if(CrtEffect* v { _CrtEffect.Resolve(flavorHashes, isStyled, part, CrtEffect {}, [](const FlavorPart& style) -> const std::optional<CrtEffect>& { return style.GetCrtEffect(); }) })
                    element.SetCrtEffect(*v);
            }
        }

        /// @brief Leaves a setting to the flavors and the default. Settings these layers don't keep are ignored
        void Reset(const StyleSetting setting)
        {
            switch(setting)
            {
                case StyleSetting::FillType:
                    _FillType.Reset();
                break;
                case StyleSetting::GradientDegree:
                    _GradientDegree.Reset();
                break;
                case StyleSetting::GradientOffset:
                    _GradientOffset.Reset();
                break;
                case StyleSetting::GradientAnimationSpeed:
                    _GradientAnimationSpeed.Reset();
                break;
                case StyleSetting::GradientRotationSpeed:
                    _GradientRotationSpeed.Reset();
                break;
                case StyleSetting::RasterPattern:
                    _RasterPattern.Reset();
                break;
                case StyleSetting::PrimaryRasterColor:
                    _PrimaryRasterColor.Reset();
                break;
                case StyleSetting::SecondaryRasterColor:
                    _SecondaryRasterColor.Reset();
                break;
                case StyleSetting::RasterSpacing:
                    _RasterSpacing.Reset();
                break;
                case StyleSetting::RasterDegree:
                    _RasterDegree.Reset();
                break;
                case StyleSetting::PrimaryRasterWidthTransfer:
                    _PrimaryRasterWidthTransfer.Reset();
                break;
                case StyleSetting::DotTransparencyTransfer:
                    _DotTransparencyTransfer.Reset();
                break;
                case StyleSetting::RasterAnimationSpeed:
                    _RasterAnimationSpeed.Reset();
                break;
                case StyleSetting::WindowImageTextureID:
                    _WindowImageTextureID.Reset();
                break;
                case StyleSetting::GlassNoise:
                    _GlassNoise.Reset();
                break;
                case StyleSetting::FogAlpha:
                    _FogAlpha.Reset();
                break;
                case StyleSetting::FogSpeed:
                    _FogSpeed.Reset();
                break;
                case StyleSetting::FogDensity:
                    _FogDensity.Reset();
                break;
                case StyleSetting::FogClearing:
                    _FogClearing.Reset();
                break;
                case StyleSetting::FogNoise:
                    _FogNoise.Reset();
                break;
                case StyleSetting::ImagePadding:
                    _ImagePadding.Reset();
                break;
                case StyleSetting::ImageFit:
                    _ImageFit.Reset();
                break;
                case StyleSetting::MaskTextureID:
                    _MaskTextureID.Reset();
                break;
                case StyleSetting::MaskMode:
                    _MaskMode.Reset();
                break;
                case StyleSetting::MaskFit:
                    _MaskFit.Reset();
                break;
                case StyleSetting::Gaps:
                    _BackgroundGaps.Reset();
                    _BorderGaps.Reset();
                break;
                case StyleSetting::PixelateBlockSize:
                    _PixelateBlockSize.Reset();
                break;
                case StyleSetting::CrtEffect:
                    _CrtEffect.Reset();
                break;
                default:
                break;
            }
        }
    };
}