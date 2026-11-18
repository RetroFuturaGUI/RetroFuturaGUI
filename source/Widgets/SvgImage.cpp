#include "SvgImage.hpp"
#include "ResourceManager.hpp"
#include <glm/common.hpp>

RetroFuturaGUI::SvgImage::SvgImage(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view SvgImagePath)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::SvgImage;
    _svgTexture = ResourceManager::CreateSvgTexture(SvgImagePath, projection);

    if(!_svgTexture)
        return;

    _svgTextureAspectRatio = _svgTexture->GetAspectRatio();
}

void RetroFuturaGUI::SvgImage::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    FitGeometryToTexture();
}

void RetroFuturaGUI::SvgImage::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);

    if(!_svgTexture)
        return;

    _svgTexture->SetPosition(position);
}

void RetroFuturaGUI::SvgImage::FitGeometryToTexture()
{
    if(!_svgTexture)
        return;

    const glm::vec2 widgetSize { _size.x, _size.y };
    _quadSize = widgetSize;

    //The size the widget's height gives at the drawing's proportions, and the one its width gives: Fit takes the one that stays
    //inside the widget, Fill the one that covers it. A drawing isn't tiled, so Repeat stretches it
    if(ImageFit::Stretch != _imageFit && ImageFit::Repeat != _imageFit && 0.0f < _svgTextureAspectRatio)
    {
        const glm::vec2
            byHeight { widgetSize.y * _svgTextureAspectRatio, widgetSize.y },
            byWidth { widgetSize.x, widgetSize.x / _svgTextureAspectRatio };
        const bool isHeightInside { byHeight.x <= widgetSize.x };
        _quadSize = (ImageFit::Fit == _imageFit) == isHeightInside ? byHeight : byWidth;
    }

    _svgTexture->SetSize(_quadSize);
}

void RetroFuturaGUI::SvgImage::SetImageFit(const ImageFit fit)
{
    _imageFit = fit;
    FitGeometryToTexture();
}

RetroFuturaGUI::ImageFit RetroFuturaGUI::SvgImage::GetImageFit() const
{
    return _imageFit;
}

void RetroFuturaGUI::SvgImage::Draw()
{
    if(!_svgTexture)
        return;

    rasterizeAtShownSize();
    _svgTexture->Draw();
}

void RetroFuturaGUI::SvgImage::rasterizeAtShownSize()
{
    if(!_svgTexture)
        return;

    const glm::i32vec2 shownSize { glm::clamp(glm::i32vec2(glm::ceil(_quadSize)), glm::i32vec2(1), glm::i32vec2(_maxRasterSide)) };

    if(shownSize == _svgTexture->GetResolution())
        return;

    _svgTexture->Rasterize(shownSize);
}

void RetroFuturaGUI::SvgImage::SetPathFill(std::string_view name, const SvgPathFill& fill)
{
    if(!_svgTexture)
        return;

    _svgTexture->SetPathFill(name, fill);
}

void RetroFuturaGUI::SvgImage::ClearPathFill(std::string_view name)
{
    if(!_svgTexture)
        return;

    _svgTexture->ClearPathFill(name);
}

const std::vector<RetroFuturaGUI::SvgPathInfo>& RetroFuturaGUI::SvgImage::GetNamedPaths() const
{
    static const std::vector<SvgPathInfo> empty;
    return _svgTexture ? _svgTexture->GetNamedPaths() : empty;
}