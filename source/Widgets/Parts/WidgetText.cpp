#include "WidgetText.hpp"
#include "ResourceManager.hpp"
#include <utility>

RetroFuturaGUI::WidgetText::WidgetText(Projection* projection, std::function<void(WidgetTextChange)> changeHook, WidgetText* follower)
    : _text(projection), _outline(_text), _changeHook(std::move(changeHook)), _follower(follower)
{
}

void RetroFuturaGUI::WidgetText::SetString(std::string_view string, const bool emitSignal)
{
    _text.SetTextUTF8(string);
    notifyChange(WidgetTextChange::String);

    if(!emitSignal)
        return;

    EmitChange();
}

const std::string& RetroFuturaGUI::WidgetText::GetString() const
{
    return _text.GetTextUTF8();
}

const char* RetroFuturaGUI::WidgetText::GetCString() const
{
    return _text.GetTextUTF8().c_str();
}

void RetroFuturaGUI::WidgetText::Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onTextChangeAsync.Connect(slot);
    else
        _onTextChange.Connect(slot);
}

void RetroFuturaGUI::WidgetText::Disconnect_OnTextChange(const typename Signal<>::Slot& slot)
{
    _onTextChange.Disconnect(slot);
    _onTextChangeAsync.Disconnect(slot);
}

void RetroFuturaGUI::WidgetText::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontFamily.Set(std::string(fontFamily));
    _fontSize.Set(FontManager::ToPixels(fontSize, sizeUnit));
    _fontSlant.Set(slant);
    _fontWeight.Set(fontWeight);
    setLayout();
}

void RetroFuturaGUI::WidgetText::SetFontSize(const f32 fontSize, const FontSizeUnit sizeUnit)
{
    _fontSize.Set(FontManager::ToPixels(fontSize, sizeUnit));
    setLayout();
}

void RetroFuturaGUI::WidgetText::SetFontSlant(const PlatformBridge::Fonts::Slant slant)
{
    _fontSlant.Set(slant);
    setLayout();
}

void RetroFuturaGUI::WidgetText::SetFontWeight(const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontWeight.Set(fontWeight);
    setLayout();
}

f32 RetroFuturaGUI::WidgetText::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return _text.GetFontSize(sizeUnit);
}

void RetroFuturaGUI::WidgetText::SetAlignment(const TextAlignment alignment)
{
    _alignment.Set(alignment);
    setLayout();
}

void RetroFuturaGUI::WidgetText::SetFlow(const TextFlow textFlow)
{
    _flow.Set(textFlow);
    setLayout();
}

void RetroFuturaGUI::WidgetText::SetPadding(const f32 padding)
{
    _padding.Set(padding);
    setLayout();
}

RetroFuturaGUI::TextAlignment RetroFuturaGUI::WidgetText::GetAlignment() const
{
    return _text.GetTextAlignment();
}

RetroFuturaGUI::TextFlow RetroFuturaGUI::WidgetText::GetFlow() const
{
    return _text.GetTextFlow();
}

void RetroFuturaGUI::WidgetText::EnableWordWrap(const bool enable)
{
    _text.EnableWordWrap(enable);
}

bool RetroFuturaGUI::WidgetText::IsWordWrapEnabled() const
{
    return _text.IsWordWrapEnabled();
}

f32 RetroFuturaGUI::WidgetText::GetPadding() const
{
    return _text.GetPadding();
}

f32 RetroFuturaGUI::WidgetText::GetWidth() const
{
    return _text.GetWidth();
}

void RetroFuturaGUI::WidgetText::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(_colors, state).assign(colors.begin(), colors.end());
    setColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::GetColors(const ColorState state) const
{
    return colorsOf(_colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::GetShownColors(const ColorState state) const
{
    const std::vector<glm::vec4>& own { colorsOf(_colors, state) };

    if(!own.empty())
        return own;

    //A text without a widget's flavors goes from its own colors straight to its built-in ones
    const std::vector<glm::vec4>* styled { _flavors ? StylingManager::FindColors(*_flavors, _stylePart, state) : nullptr };

    if(styled)
        return *styled;

    const std::vector<glm::vec4>& builtIn { colorsOf(_builtInColors, state) };

    if(!builtIn.empty() || ColorState::Enabled == state)
        return builtIn;

    return GetShownColors(ColorState::Enabled); //a state nothing sets, such as Selected, looks like Enabled
}

void RetroFuturaGUI::WidgetText::SetBuiltInColors(const StyleKey&, std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(_builtInColors, state).assign(colors.begin(), colors.end());
    setColors();
}

void RetroFuturaGUI::WidgetText::SetFillType(const FillType fillType)
{
    _shader._FillType.Set(fillType);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetGradientOffset(const f32 gradientOffset)
{
    _shader._GradientOffset.Set(gradientOffset);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _shader._GradientAnimationSpeed.Set(animationSpeed);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetGradientDegree(const f32 degree)
{
    _shader._GradientDegree.Set(degree);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _shader._GradientRotationSpeed.Set(rotationSpeed);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetColorGlyphTint(const f32 tint)
{
    _colorGlyphTint.Set(tint);
    setLayout();
}

void RetroFuturaGUI::WidgetText::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    _shader._WindowImageTextureID.Set(textureID);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetImageTextureID(const u32 textureID)
{
    _imageTextureID.Set(textureID);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetImagePadding(const f32 padding)
{
    _shader._ImagePadding.Set(padding);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetImageFit(const ImageFit fit)
{
    _shader._ImageFit.Set(fit);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetMaskTextureID(const u32 textureID)
{
    _shader._MaskTextureID.Set(textureID);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetMaskMode(const MaskMode mode)
{
    _shader._MaskMode.Set(mode);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetMaskFit(const ImageFit fit)
{
    _shader._MaskFit.Set(fit);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetMaskSpan(const MaskSpan maskSpan)
{
    _maskSpan.Set(maskSpan);
    setLayout();
}

void RetroFuturaGUI::WidgetText::SetRasterPattern(const RasterPattern rasterPattern)
{
    _shader._RasterPattern.Set(rasterPattern);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetPrimaryRasterColor(const glm::vec4& color)
{
    _shader._PrimaryRasterColor.Set(color);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetSecondaryRasterColor(const glm::vec4& color)
{
    _shader._SecondaryRasterColor.Set(color);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetRasterSpacing(const f32 spacing)
{
    _shader._RasterSpacing.Set(spacing);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetRasterDegree(const f32 degree)
{
    _shader._RasterDegree.Set(degree);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetPrimaryRasterWidthTransfer(std::span<const f32> widthTransfer)
{
    _shader._PrimaryRasterWidthTransfer.Set(std::vector<f32>(widthTransfer.begin(), widthTransfer.end()));
    setShader();
}

void RetroFuturaGUI::WidgetText::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _shader._DotTransparencyTransfer.Set(transparencyTransfer);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    _shader._RasterAnimationSpeed.Set(animationSpeed);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetFogAlpha(const f32 alpha)
{
    _shader._FogAlpha.Set(alpha);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetFogSpeed(const f32 speed)
{
    _shader._FogSpeed.Set(speed);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetFogDensity(std::span<const f32> density)
{
    _shader._FogDensity.Set(std::vector<f32>(density.begin(), density.end()));
    setShader();
}

void RetroFuturaGUI::WidgetText::SetFogClearing(const f32 clearing)
{
    _shader._FogClearing.Set(clearing);
    setShader();
}

void RetroFuturaGUI::WidgetText::Draw()
{
    followStyles();
    _text.Draw();
}

void RetroFuturaGUI::WidgetText::SetSize(const glm::vec2& size)
{
    _text.SetParentSize(size);
}

void RetroFuturaGUI::WidgetText::SetPosition(const glm::vec3& position)
{
    _text.SetPosition(position);
}

void RetroFuturaGUI::WidgetText::SetColorState(const ColorState state)
{
    _colorState = state;
    setColors();
}

void RetroFuturaGUI::WidgetText::SetTransitionSettings(const TransitionSettings* settings)
{
    _text.SetTransitionSettings(settings);
}

void RetroFuturaGUI::WidgetText::EmitChange()
{
    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

RetroFuturaGUI::Outline& RetroFuturaGUI::WidgetText::GetOutline()
{
    return _outline;
}

const RetroFuturaGUI::Outline& RetroFuturaGUI::WidgetText::GetOutline() const
{
    return _outline;
}

RetroFuturaGUI::Text& RetroFuturaGUI::WidgetText::GetElement()
{
    return _text;
}

const RetroFuturaGUI::Text& RetroFuturaGUI::WidgetText::GetElement() const
{
    return _text;
}

std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::colorsOf(StateColors& colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            return colors._Clicked;
        case ColorState::Disabled:
            return colors._Disabled;
        case ColorState::Hover:
            return colors._Hover;
        case ColorState::Selected:
            return colors._Selected;
        default: // Enabled
            return colors._Enabled;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::colorsOf(const StateColors& colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            return colors._Clicked;
        case ColorState::Disabled:
            return colors._Disabled;
        case ColorState::Hover:
            return colors._Hover;
        case ColorState::Selected:
            return colors._Selected;
        default: // Enabled
            return colors._Enabled;
    }
}

void RetroFuturaGUI::WidgetText::setStyleSource(const std::vector<u32>* flavors, const StylePart part)
{
    _flavors = flavors;
    _stylePart = part;
    _styleGeneration = StylingManager::GetGeneration();
    setLayout();
    setColors();
    setShader();
    restyleEffects();
}

void RetroFuturaGUI::WidgetText::followStyles()
{
    if(StylingManager::GetGeneration() == _styleGeneration)
        return;

    //The colors are handed on by reference, so they go again even when unchanged. A flavor's list may have moved
    _styleGeneration = StylingManager::GetGeneration();
    setLayout();
    setColors();
    setShader();
    restyleEffects();
}

void RetroFuturaGUI::WidgetText::setColors()
{
    _text.SetColors(GetShownColors(_colorState));
    _outline.SetColorState(_colorState);
    _text.GetDropShadow().SetColorState(_colorState);
    _text.GetGlow().SetColorState(_colorState);
    _text.GetInnerGlow().SetColorState(_colorState);
}

void RetroFuturaGUI::WidgetText::notifyChange(const WidgetTextChange change)
{
    if(!_changeHook)
        return;

    _changeHook(change);
}

RetroFuturaGUI::DropShadow& RetroFuturaGUI::WidgetText::GetDropShadow()
{
    return _text.GetDropShadow();
}

const RetroFuturaGUI::DropShadow& RetroFuturaGUI::WidgetText::GetDropShadow() const
{
    return _text.GetDropShadow();
}

RetroFuturaGUI::Glow& RetroFuturaGUI::WidgetText::GetGlow()
{
    return _text.GetGlow();
}

const RetroFuturaGUI::Glow& RetroFuturaGUI::WidgetText::GetGlow() const
{
    return _text.GetGlow();
}

RetroFuturaGUI::InnerGlow& RetroFuturaGUI::WidgetText::GetInnerGlow()
{
    return _text.GetInnerGlow();
}

const RetroFuturaGUI::InnerGlow& RetroFuturaGUI::WidgetText::GetInnerGlow() const
{
    return _text.GetInnerGlow();
}

void RetroFuturaGUI::WidgetText::SetGlassNoise(const GlassNoise& noise)
{
    _shader._GlassNoise.Set(noise);
    setShader();
}

void RetroFuturaGUI::WidgetText::SetFogNoise(const FogNoise& noise)
{
    _shader._FogNoise.Set(noise);
    setShader();
}

void RetroFuturaGUI::WidgetText::setShader()
{
    const std::span<const u32> flavors { getFlavors() };
    const bool isStyled { nullptr != _flavors };

    _shader.Apply(_text, flavors, isStyled, _stylePart);

    if(const u32* textureID { _imageTextureID.Resolve(flavors, isStyled, _stylePart, 0u, &WidgetText::imageTextureOf) })
        _text.SetBackgroundImageTextureID(*textureID);
}

void RetroFuturaGUI::WidgetText::setLayout()
{
    const std::span<const u32> flavors { getFlavors() };
    const bool isStyled { nullptr != _flavors };

    layoutFont();
    layoutAlignment();
    layoutPadding();

    if(const TextFlow* flow { _flow.Resolve(flavors, isStyled, _stylePart, TextFlow::Horizontal, [](const FlavorPart& part) -> const std::optional<TextFlow>& { return part.GetFlow(); }) })
    {
        _text.SetTextFlow(*flow);
        notifyChange(WidgetTextChange::Flow);
    }

    if(const f32* tint { _colorGlyphTint.Resolve(flavors, isStyled, _stylePart, 0.0f, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetColorGlyphTint(); }) })
        _text.SetColorGlyphTint(*tint);

    if(const MaskSpan* maskSpan { _maskSpan.Resolve(flavors, isStyled, _stylePart, MaskSpan::WholeText, [](const FlavorPart& part) -> const std::optional<MaskSpan>& { return part.GetMaskSpan(); }) })
        _text.SetMaskSpan(*maskSpan);
}

void RetroFuturaGUI::WidgetText::layoutFont()
{
    const std::span<const u32> flavors { getFlavors() };
    const bool isStyled { nullptr != _flavors };

    //Each goes through the layers, so all four keep up even while the text has no family yet.
    const bool isFamilyChanged { nullptr != _fontFamily.Resolve(flavors, isStyled, _stylePart, std::string {}, [](const FlavorPart& part) -> const std::optional<std::string>& { return part.GetFontFamily(); }) };
    const bool isSizeChanged { nullptr != _fontSize.Resolve(flavors, isStyled, _stylePart, _defaultFontSize, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetFontSize(); }) };
    const bool isSlantChanged { nullptr != _fontSlant.Resolve(flavors, isStyled, _stylePart, PlatformBridge::Fonts::Slant::Roman, [](const FlavorPart& part) -> const std::optional<PlatformBridge::Fonts::Slant>& { return part.GetFontSlant(); }) };
    const bool isWeightChanged { nullptr != _fontWeight.Resolve(flavors, isStyled, _stylePart, PlatformBridge::Fonts::Weight::Normal, [](const FlavorPart& part) -> const std::optional<PlatformBridge::Fonts::Weight>& { return part.GetFontWeight(); }) };

    if(!isFamilyChanged && !isSizeChanged && !isSlantChanged && !isWeightChanged)
        return;

    const std::optional<std::string>& family { _fontFamily.GetShown() };

    if(!family || family->empty())
        return; //a text without a family draws nothing, so the rest waits for one.

    const f32 size { _fontSize.GetShown().value_or(_defaultFontSize) };
    const PlatformBridge::Fonts::Slant slant { _fontSlant.GetShown().value_or(PlatformBridge::Fonts::Slant::Roman) };
    const PlatformBridge::Fonts::Weight weight { _fontWeight.GetShown().value_or(PlatformBridge::Fonts::Weight::Normal) };

    _text.SetFontFamily(*family, size, FontSizeUnit::Pixels, slant, weight);
    notifyChange(WidgetTextChange::Font);

    if(!_follower)
        return;

    _follower->SetFontFamily(*family, size, FontSizeUnit::Pixels, slant, weight);
}

void RetroFuturaGUI::WidgetText::layoutAlignment()
{
    const TextAlignment* alignment { _alignment.Resolve(getFlavors(), nullptr != _flavors, _stylePart, TextAlignment::Left, [](const FlavorPart& part) -> const std::optional<TextAlignment>& { return part.GetAlignment(); }) };

    if(!alignment)
        return;

    _text.SetTextAlignment(*alignment);
    notifyChange(WidgetTextChange::Alignment);

    if(!_follower)
        return;

    _follower->SetAlignment(*alignment);
}

void RetroFuturaGUI::WidgetText::layoutPadding()
{
    const f32* padding { _padding.Resolve(getFlavors(), nullptr != _flavors, _stylePart, 0.0f, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetPadding(); }) };

    if(!padding)
        return;

    _text.SetTextPadding(*padding);
    notifyChange(WidgetTextChange::Padding);

    if(!_follower)
        return;

    _follower->SetPadding(*padding);
}

std::span<const u32> RetroFuturaGUI::WidgetText::getFlavors() const
{
    if(!_flavors)
        return {};

    return *_flavors;
}

std::optional<u32> RetroFuturaGUI::WidgetText::imageTextureOf(const FlavorPart& part)
{
    const std::optional<std::string>& path { part.GetImage() };

    if(!path)
        return std::nullopt;

    if(path->empty())
        return 0u; //set to show none.

    const Texture* texture { ResourceManager::GetTexture(*path) };

    if(!texture)
        return 0u;

    return texture->GetID();
}

void RetroFuturaGUI::WidgetText::ResetSetting(const StyleSetting setting)
{
    switch(setting)
    {
        case StyleSetting::FontFamily:
            _fontFamily.Reset();
        break;
        case StyleSetting::FontSize:
            _fontSize.Reset();
        break;
        case StyleSetting::FontSlant:
            _fontSlant.Reset();
        break;
        case StyleSetting::FontWeight:
            _fontWeight.Reset();
        break;
        case StyleSetting::Alignment:
            _alignment.Reset();
        break;
        case StyleSetting::Padding:
            _padding.Reset();
        break;
        case StyleSetting::Flow:
            _flow.Reset();
        break;
        case StyleSetting::ColorGlyphTint:
            _colorGlyphTint.Reset();
        break;
        case StyleSetting::MaskSpan:
            _maskSpan.Reset();
        break;
        case StyleSetting::Image:
            _imageTextureID.Reset();
        break;
        default:
            _shader.Reset(setting);
        break;
    }

    setLayout();
    setShader();
}

void RetroFuturaGUI::WidgetText::restyleEffects()
{
    _outline.followFlavors(_flavors, _stylePart);
    _text.GetDropShadow().followFlavors(_flavors, _stylePart);
    _text.GetGlow().followFlavors(_flavors, _stylePart);
    _text.GetInnerGlow().followFlavors(_flavors, _stylePart);
}