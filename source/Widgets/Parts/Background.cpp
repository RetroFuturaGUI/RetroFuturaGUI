#include "Background.hpp"
#include "Border.hpp"
#include "IWidgetDecoration.hpp"
#include "IncludeHelper.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <cctype>
#include <memory>
#include <utility>

RetroFuturaGUI::Background::Background(Projection* projection, BackgroundHooks hooks)
    : IWidgetDecoration(projection, std::move(hooks._OnCornerRadiiChange))
{
    _backgroundPlane = std::make_unique<Plane>(_projection);
    _shadowRectangle = std::make_unique<ShadowRectangle>(_projection, &_dropShadow);
    _glowRectangle = std::make_unique<GlowRectangle>(_projection, &_glow);
    _innerGlowRectangle = std::make_unique<InnerGlowRectangle>(_projection, &_innerGlow);
    setColors();
}

void RetroFuturaGUI::Background::Draw()
{
    if(!_backgroundPlane)
        return;

    followStyles();
    drawShadow();
    drawBorderGlow(false); //under the fill, showing in the gap

    if(!isGlowDrawnByBorder())
        drawGlow(_backgroundPlane->GetPosition().z - _glowDepthOffset);

    _backgroundPlane->Draw();
    drawSvg();
    drawInnerGlow();
    drawBorderGlow(true);
}

void RetroFuturaGUI::Background::setCornerShape()
{
    if(!_backgroundPlane)
        return;

    _backgroundPlane->SetCornerRadii(_cornerRadii);
    _backgroundPlane->SetCutCorners(_cutCorners);
}

void RetroFuturaGUI::Background::SetTransitionSettings(const TransitionSettings* settings)
{
    if(!_backgroundPlane)
        return;

    _backgroundPlane->SetTransitionSettings(settings);
    _dropShadow.SetTransitionSettings(settings);
    _glow.SetTransitionSettings(settings);
    _innerGlow.SetTransitionSettings(settings);
}

void RetroFuturaGUI::Background::SetSize(const glm::vec2& size)
{
    _size = size;
    applyInset();
}

void RetroFuturaGUI::Background::SetPosition(const glm::vec3& position)
{
    _position = position;
    applyInset();
}

void RetroFuturaGUI::Background::SetRotation(const glm::vec3& rotation)
{
    if(!_backgroundPlane)
        return;

    _backgroundPlane->SetRotation(rotation);
}

glm::vec2 RetroFuturaGUI::Background::GetSize() const
{
    return _size;
}

glm::vec3 RetroFuturaGUI::Background::GetPosition() const
{
    return _position;
}

glm::vec3 RetroFuturaGUI::Background::GetRotation() const
{
    if(!_backgroundPlane)
        return glm::vec3(0.0f);

    return _backgroundPlane->GetRotation();
}

void RetroFuturaGUI::Background::SetGradientOffset(const f32 gradientOffset)
{
    _shader._GradientOffset.Set(gradientOffset);
    setShader();
}

void RetroFuturaGUI::Background::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _shader._GradientAnimationSpeed.Set(animationSpeed);
    setShader();
}

void RetroFuturaGUI::Background::SetGradientDegree(const f32 degree)
{
    _shader._GradientDegree.Set(degree);
    setShader();
}

void RetroFuturaGUI::Background::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _shader._GradientRotationSpeed.Set(rotationSpeed);
    setShader();
}

const std::vector<glm::vec4>& RetroFuturaGUI::Background::GetColors(const ColorState state) const
{
    return getOwnColors(state);
}

void RetroFuturaGUI::Background::SetFillType(const FillType fillType)
{
    _shader._FillType.Set(fillType);
    setShader();
}

void RetroFuturaGUI::Background::SetGaps(const BackgroundGap& gap)
{
    _shader._BackgroundGaps.Set(gap);
    setShader();
}

void RetroFuturaGUI::Background::SetWindowImageTextureID(const u32 textureID)
{
    _shader._WindowImageTextureID.Set(textureID);
    setShader();
}

void RetroFuturaGUI::Background::SetRasterPattern(const RasterPattern rasterPattern)
{
    _shader._RasterPattern.Set(rasterPattern);
    setShader();
}

void RetroFuturaGUI::Background::SetPrimaryRasterColor(const glm::vec4& color)
{
    _shader._PrimaryRasterColor.Set(color);
    setShader();
}

void RetroFuturaGUI::Background::SetSecondaryRasterColor(const glm::vec4& color)
{
    _shader._SecondaryRasterColor.Set(color);
    setShader();
}

void RetroFuturaGUI::Background::SetRasterSpacing(const f32 spacing)
{
    _shader._RasterSpacing.Set(spacing);
    setShader();
}

void RetroFuturaGUI::Background::SetRasterDegree(const f32 degree)
{
    _shader._RasterDegree.Set(degree);
    setShader();
}

void RetroFuturaGUI::Background::SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _shader._PrimaryRasterWidthTransfer.Set(std::vector(widthTransfer.begin(), widthTransfer.end()));
    setShader();
}

void RetroFuturaGUI::Background::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _shader._DotTransparencyTransfer.Set(transparencyTransfer);
    setShader();
}

void RetroFuturaGUI::Background::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    _shader._RasterAnimationSpeed.Set(animationSpeed);
    setShader();
}

void RetroFuturaGUI::Background::SetFogAlpha(const f32 alpha)
{
    _shader._FogAlpha.Set(alpha);
    setShader();
}

void RetroFuturaGUI::Background::SetFogSpeed(const f32 speed)
{
    _shader._FogSpeed.Set(speed);
    setShader();
}

void RetroFuturaGUI::Background::SetFogDensity(std::span<f32> density)
{
    _shader._FogDensity.Set(std::vector(density.begin(), density.end()));
    setShader();
}

void RetroFuturaGUI::Background::SetFogClearing(const f32 clearing)
{
    _shader._FogClearing.Set(clearing);
    setShader();
}

void RetroFuturaGUI::Background::SetImage(std::string_view imagePath)
{
    _image.Set(std::string(imagePath));
    setShader();
}

void RetroFuturaGUI::Background::loadImage(std::string_view imagePath)
{
    if(!_backgroundPlane)
        return;

    if(imagePath.empty())
    {
        _backgroundSvg.reset();
        _backgroundImage = nullptr;
        _backgroundPlane->SetBackgroundImageTextureID(0);
        return;
    }

    if(isSvgPath(imagePath))
    {
        _backgroundImage = nullptr;
        _backgroundPlane->SetBackgroundImageTextureID(0);
        _backgroundSvg = ResourceManager::CreateSvgTexture(imagePath, &_backgroundPlane->GetProjection());
        return;
    }

    _backgroundSvg.reset();
    _backgroundImage = ResourceManager::GetTexture(imagePath);

    if(!_backgroundImage)
        return;

    _backgroundPlane->SetBackgroundImageTextureID(_backgroundImage->GetID());
}

u32 RetroFuturaGUI::Background::GetImageTextureID() const
{
    if(!_backgroundImage)
        return 0;

    return _backgroundImage->GetID();
}

void RetroFuturaGUI::Background::SetImagePadding(const f32 padding)
{
    _shader._ImagePadding.Set(padding);
    setShader();
}

void RetroFuturaGUI::Background::SetImageFit(const ImageFit fit)
{
    _shader._ImageFit.Set(fit);
    setShader();
}

void RetroFuturaGUI::Background::SetMaskTextureID(const u32 textureID)
{
    _shader._MaskTextureID.Set(textureID);
    setShader();
}

void RetroFuturaGUI::Background::SetMaskMode(const MaskMode mode)
{
    _shader._MaskMode.Set(mode);
    setShader();
}

void RetroFuturaGUI::Background::SetMaskFit(const ImageFit fit)
{
    _shader._MaskFit.Set(fit);
    setShader();
}

void RetroFuturaGUI::Background::SetImagePathFill(std::string_view name, const SvgPathFill& fill)
{
    if(!_backgroundSvg)
        return;

    _backgroundSvg->SetPathFill(name, fill);
}

void RetroFuturaGUI::Background::ClearImagePathFill(std::string_view name)
{
    if(!_backgroundSvg)
        return;

    _backgroundSvg->ClearPathFill(name);
}

void RetroFuturaGUI::Background::setColors()
{
    if(!_backgroundPlane)
        return;

    _backgroundPlane->SetColors(GetShownColors(_colorState));

    _dropShadow.SetColorState(_colorState);
    _glow.SetColorState(_colorState);
    _innerGlow.SetColorState(_colorState);
}

void RetroFuturaGUI::Background::drawSvg()
{
    if(!_backgroundPlane)
        return;

    if(!_backgroundSvg)
        return;

    //Widgets place _backgroundPlane themselves, so the SVG follows it here. SvgTexture rebuilds a matrix only for a value that changed
    const glm::vec2 size { glm::max(_backgroundPlane->GetSize() - glm::vec2(_shader._ImagePadding.GetShown().value_or(0.0f) * 2.0f), glm::vec2(0.0f)) };
    _backgroundSvg->SetSize(size);
    _backgroundSvg->SetPosition(_backgroundPlane->GetPosition() + glm::vec3(0.0f, 0.0f, 0.005f));
    _backgroundSvg->SetRotation(_backgroundPlane->GetRotation());
    _backgroundSvg->Draw();
}

bool RetroFuturaGUI::Background::isSvgPath(std::string_view path)
{
    constexpr std::string_view extension { ".svg" };

    if(path.size() < extension.size())
        return false;

    const std::string_view end { path.substr(path.size() - extension.size()) };

    return std::equal(end.begin(), end.end(), extension.begin(), [](const char left, const char right)
        {
            return std::tolower(static_cast<unsigned char>(left)) == right;
        });
}

void RetroFuturaGUI::Background::setInset(const glm::vec4& inset)
{
    _inset = inset;
    applyInset();
}

void RetroFuturaGUI::Background::applyInset()
{
    if(!_backgroundPlane)
        return;

    //The inset runs top, right, bottom, left, and y runs up, so a larger top inset moves the fill down
    const glm::vec2
        size { glm::max(_size - glm::vec2(_inset.y + _inset.w, _inset.x + _inset.z), glm::vec2(0.0f)) },
        shift { (_inset.w - _inset.y) * 0.5f, (_inset.z - _inset.x) * 0.5f };

    _backgroundPlane->SetSize(size);
    _backgroundPlane->SetPosition(_position + glm::vec3(shift, 0.0f));
}

RetroFuturaGUI::DropShadow& RetroFuturaGUI::Background::GetDropShadow()
{
    return _dropShadow;
}

const RetroFuturaGUI::DropShadow& RetroFuturaGUI::Background::GetDropShadow() const
{
    return _dropShadow;
}

void RetroFuturaGUI::Background::drawShadow()
{
    if(!_backgroundPlane)
        return;

    if(!_shadowRectangle)
        return;

    if(!_dropShadow.IsShown())
        return;

    _shadowRectangle->SetCaster(ShadowCaster
    {
        ._Size = _backgroundPlane->GetSize(),
        ._Position = _backgroundPlane->GetPosition(),
        ._CornerRadii = _backgroundPlane->GetShownCornerRadii(),
        ._CutCorners = _backgroundPlane->GetCutCorners(),
        ._RingWidth = 0.0f
    });
    _shadowRectangle->Draw();
}

RetroFuturaGUI::Glow& RetroFuturaGUI::Background::GetGlow()
{
    return _glow;
}

const RetroFuturaGUI::Glow& RetroFuturaGUI::Background::GetGlow() const
{
    return _glow;
}

void RetroFuturaGUI::Background::EnableGlowOverBorder(const bool enable)
{
    _isGlowOverBorder = enable;
}

bool RetroFuturaGUI::Background::IsGlowOverBorderEnabled() const
{
    return _isGlowOverBorder;
}

void RetroFuturaGUI::Background::drawGlow(const f32 depth)
{
    if(!_backgroundPlane)
        return;

    if(!_glowRectangle)
        return;

    if(!_glow.IsShown())
        return;

    const glm::vec3 position { glm::vec2(_backgroundPlane->GetPosition()), depth };
    _glowRectangle->SetCaster(ShadowCaster
    {
        ._Size = _backgroundPlane->GetSize(),
        ._Position = position,
        ._CornerRadii = _backgroundPlane->GetShownCornerRadii(),
        ._CutCorners = _backgroundPlane->GetCutCorners(),
        ._RingWidth = 0.0f
    }, GlowEdge::Outer);
    _glowRectangle->Draw();
}

void RetroFuturaGUI::Background::drawBorderGlow(const bool isOver)
{
    if(!_backgroundPlane)
        return;

    if(!_linkedBorder)
        return;

    if(isOver != _linkedBorder->_isGlowOverBackground)
        return;

    _linkedBorder->drawInwardGlow(_backgroundPlane->GetPosition().z + (isOver ? _coverDepthOffset : -_coverDepthOffset));
}

bool RetroFuturaGUI::Background::isGlowDrawnByBorder() const
{
    return _linkedBorder != nullptr && _isGlowOverBorder;
}

RetroFuturaGUI::InnerGlow& RetroFuturaGUI::Background::GetInnerGlow()
{
    return _innerGlow;
}

const RetroFuturaGUI::InnerGlow& RetroFuturaGUI::Background::GetInnerGlow() const
{
    return _innerGlow;
}

void RetroFuturaGUI::Background::drawInnerGlow()
{
    if(!_backgroundPlane)
        return;

    if(!_innerGlowRectangle)
        return;

    if(!_innerGlow.IsShown())
        return;

    _innerGlowRectangle->SetCaster(ShadowCaster
    {
        ._Size = _backgroundPlane->GetSize(),
        ._Position = _backgroundPlane->GetPosition() + glm::vec3(0.0f, 0.0f, _innerGlowDepthOffset),
        ._CornerRadii = _backgroundPlane->GetShownCornerRadii(),
        ._CutCorners = _backgroundPlane->GetCutCorners(),
        ._RingWidth = 0.0f
    });
    _innerGlowRectangle->Draw();
}

void RetroFuturaGUI::Background::SetGlassNoise(const GlassNoise& noise)
{
    _shader._GlassNoise.Set(noise);
    setShader();
}

void RetroFuturaGUI::Background::SetFogNoise(const FogNoise& noise)
{
    _shader._FogNoise.Set(noise);
    setShader();
}

void RetroFuturaGUI::Background::SetPixelateBlockSize(const f32 blockSize)
{
    _shader._PixelateBlockSize.Set(blockSize);
    setShader();
}

void RetroFuturaGUI::Background::SetCrtEffect(const CrtEffect& crtEffect)
{
    _shader._CrtEffect.Set(crtEffect);
    setShader();
}

void RetroFuturaGUI::Background::setShader()
{
    if(!_backgroundPlane)
        return;

    _shader.Apply(*_backgroundPlane, getFlavors(), nullptr != _flavors, _stylePart);

    //The image goes by its path: loading it is the background's job, not its element's.
    if(std::string* path { _image.Resolve(getFlavors(), nullptr != _flavors, _stylePart, std::string {}, [](const FlavorPart& style) -> const std::optional<std::string>& { return style.GetImage(); }) })
        loadImage(*path);
}

bool RetroFuturaGUI::Background::resetPartSetting(const StyleSetting setting)
{
    if(StyleSetting::Image != setting)
        return false;

    _image.Reset();
    setShader();
    return true;
}

void RetroFuturaGUI::Background::restyle()
{
    IWidgetDecoration::restyle();
    _dropShadow.followFlavors(_flavors, _stylePart);
    _glow.followFlavors(_flavors, _stylePart);
    _innerGlow.followFlavors(_flavors, _stylePart);
}