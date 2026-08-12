#include "SvgTexture.hpp"
#include "ShaderManager.hpp"
#include <cstring>
#include <print>
#include <utility>

RetroFuturaGUI::SvgTexture::SvgTexture(std::string_view path, const bool flipVertically, Projection* projection,
                                        glm::i32vec2 initialResolution)
    : _verticallyFlipped(flipVertically), _projection(projection)
{
    loadDocument(path);

    if (!_document)
        return;

    discoverNamedPaths();

    const glm::i32vec2 targetResolution = (initialResolution.x > 0 && initialResolution.y > 0)
        ? initialResolution
        : glm::i32vec2(_documentIntrinsicSize);

    rasterizeBaseLayer(targetResolution);

    if (_projection)
        setupQuad();
}

RetroFuturaGUI::SvgTexture::SvgTexture(SvgTexture&& other) noexcept
    : _document(std::move(other._document)), _documentIntrinsicSize(other._documentIntrinsicSize),
      _layers(std::move(other._layers)), _pathInfoCache(std::move(other._pathInfoCache)),
      _baseLayerTextureId(other._baseLayerTextureId), _verticallyFlipped(other._verticallyFlipped),
      _resolution(other._resolution), _aspectRatio(other._aspectRatio), _path(std::move(other._path)),
      _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo), _projection(other._projection),
      _scalingMatrix(other._scalingMatrix), _translationMatrix(other._translationMatrix), _rotationMatrix(other._rotationMatrix)
{
    other._baseLayerTextureId = 0;
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
}

RetroFuturaGUI::SvgTexture& RetroFuturaGUI::SvgTexture::operator=(SvgTexture&& other) noexcept
{
    if (this == &other)
        return *this;

    releaseGPUResources();

    _document = std::move(other._document);
    _documentIntrinsicSize = other._documentIntrinsicSize;
    _layers = std::move(other._layers);
    _pathInfoCache = std::move(other._pathInfoCache);
    _baseLayerTextureId = other._baseLayerTextureId;
    _verticallyFlipped = other._verticallyFlipped;
    _resolution = other._resolution;
    _aspectRatio = other._aspectRatio;
    _path = std::move(other._path);
    _vao = other._vao;
    _vbo = other._vbo;
    _ebo = other._ebo;
    _projection = other._projection;
    _scalingMatrix = other._scalingMatrix;
    _translationMatrix = other._translationMatrix;
    _rotationMatrix = other._rotationMatrix;

    other._baseLayerTextureId = 0;
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;

    return *this;
}

RetroFuturaGUI::SvgTexture::~SvgTexture()
{
    releaseGPUResources();
}

void RetroFuturaGUI::SvgTexture::releaseGPUResources()
{
    glDeleteTextures(1, &_baseLayerTextureId);

    for (PathLayer& layer : _layers)
        glDeleteTextures(1, &layer.maskTextureId);

    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::SvgTexture::Draw()
{
    if (!_projection || _baseLayerTextureId == 0)
        return;

    ShaderManager::GetTextureFillShader().UseProgram();
    ShaderManager::GetTextureFillShader().SetUniformInt("uTexture", 0);
    ShaderManager::GetTextureFillShader().SetUniformMat4("uProjection", _projection->GetProjectionMatrix());
    ShaderManager::GetTextureFillShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetTextureFillShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetTextureFillShader().SetUniformMat4("uRotation", _rotationMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _baseLayerTextureId);

    glBindVertexArray(_vao);

    // The base layer and every masked overlay share the exact same transform, so they land at
    // identical depth. With the default GL_LESS depth func, anything after the first draw would
    // fail the depth test and be silently discarded before blending - depth testing is meaningless
    // between coplanar passes of the same widget, so it's disabled for the duration of this call.
    glDisable(GL_DEPTH_TEST);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    for (PathLayer& layer : _layers)
        if (layer.hasActiveOverride && layer.maskTextureId != 0)
            drawMaskedLayer(layer);

    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
}

void RetroFuturaGUI::SvgTexture::drawMaskedLayer(PathLayer& layer)
{
    if (layer.fill.colors.empty())
        return;

    Shader* shader = nullptr;
    switch (layer.fill.fillType)
    {
        case FillType::LINEAR_GRADIENT:
            shader = &ShaderManager::GetSvgMaskLinearGradientShader();
            break;
        case FillType::RADIAL_GRADIENT:
            shader = &ShaderManager::GetSvgMaskRadialGradientShader();
            break;
        case FillType::HUESTAR_GRADIENT:
            shader = &ShaderManager::GetSvgMaskHueStarGradientShader();
            break;
        default: // FillType::SOLID
            shader = &ShaderManager::GetSvgMaskSolidFillShader();
            break;
    }

    shader->UseProgram();
    shader->SetUniformMat4("uProjection", _projection->GetProjectionMatrix());
    shader->SetUniformMat4("uPosition", _translationMatrix);
    shader->SetUniformMat4("uScaling", _scalingMatrix);
    shader->SetUniformMat4("uRotation", _rotationMatrix);

    if (layer.fill.fillType == FillType::SOLID)
    {
        shader->SetUniformVec4("uColor", layer.fill.colors[0]);
    }
    else
    {
        layer.fill.gradientOffset += layer.fill.gradientAnimationSpeed;
        if (layer.fill.gradientOffset > 1.0f)
            layer.fill.gradientOffset = 0.0f;

        layer.fill.gradientDegree += layer.fill.gradientRotationSpeed;
        if (layer.fill.gradientDegree >= 360.0f)
            layer.fill.gradientDegree = 0.0f;

        shader->SetUniformVec4("uColors", &layer.fill.colors[0][0], static_cast<u32>(layer.fill.colors.size()));
        shader->SetUniformInt("uNumColors", static_cast<i32>(layer.fill.colors.size()));
        shader->SetUniformFloat("uDegree", layer.fill.gradientDegree);
        shader->SetUniformFloat("uGradientOffset", layer.fill.gradientOffset);
    }

    shader->SetUniformInt("uMask", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, layer.maskTextureId);
    glActiveTexture(GL_TEXTURE0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void RetroFuturaGUI::SvgTexture::SetSize(const glm::vec2& size)
{
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
}

void RetroFuturaGUI::SvgTexture::SetPosition(const glm::vec3& position)
{
    _translationMatrix = glm::translate(glm::mat4(1.0f), position);
}

void RetroFuturaGUI::SvgTexture::SetRotation(const glm::vec3& rotation)
{
    glm::vec3 radians = glm::radians(rotation);
    _rotationMatrix =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

void RetroFuturaGUI::SvgTexture::Rasterize(glm::i32vec2 targetResolution)
{
    if (!_document)
        return;

    rasterizeBaseLayer(targetResolution);

    for (PathLayer& layer : _layers)
        if (layer.maskTextureId != 0)
            materializeMask(layer, targetResolution);
}

const std::vector<RetroFuturaGUI::SvgPathInfo>& RetroFuturaGUI::SvgTexture::GetNamedPaths() const
{
    return _pathInfoCache;
}

void RetroFuturaGUI::SvgTexture::SetPathFill(std::string_view name, const SvgPathFill& fill)
{
    PathLayer* layer = findLayer(name);
    if (!layer)
    {
        std::println("SvgTexture::SetPathFill: no path named '{}'", name);
        return;
    }

    layer->fill = fill;
    layer->hasActiveOverride = true;

    if (layer->maskTextureId == 0)
        materializeMask(*layer, _resolution);
}

void RetroFuturaGUI::SvgTexture::ClearPathFill(std::string_view name)
{
    PathLayer* layer = findLayer(name);
    if (layer)
        layer->hasActiveOverride = false;
}

glm::i32vec2 RetroFuturaGUI::SvgTexture::GetResolution() const
{
    return _resolution;
}

f32 RetroFuturaGUI::SvgTexture::GetAspectRatio() const
{
    return _aspectRatio;
}

bool RetroFuturaGUI::SvgTexture::IsTextureVerticallyFlipped() const
{
    return _verticallyFlipped;
}

u32 RetroFuturaGUI::SvgTexture::GetBaseLayerTextureID() const
{
    return _baseLayerTextureId;
}

void RetroFuturaGUI::SvgTexture::loadDocument(std::string_view path)
{
    _path = path;
    _document = lunasvg::Document::loadFromFile(std::string(path));

    if (!_document)
    {
        std::println("Error loading SVG: {}", path);
        return;
    }

    _document->forceLayout();
    _documentIntrinsicSize = { _document->width(), _document->height() };
}

void RetroFuturaGUI::SvgTexture::discoverNamedPaths()
{
    // Restricted to <path> elements specifically (via a CSS tag selector, since lunasvg's Element
    // has no tagName() accessor to filter on directly) rather than walking every id-bearing node.
    // Structural/metadata elements (<svg>, <defs>, <g> layer wrappers, <image>, etc.) commonly
    // carry ids too - especially in Inkscape output - and aren't meaningful "named paths" to expose.
    for (const lunasvg::Element& element : _document->querySelectorAll("path"))
    {
        if (!element.hasAttribute("id"))
            continue;

        const std::string& id = element.getAttribute("id");
        if (id.empty())
            continue;

        lunasvg::Box bounds = element.getGlobalBoundingBox();
        _layers.push_back(PathLayer{ id, element });
        _pathInfoCache.push_back(SvgPathInfo{ id, glm::vec4(bounds.x, bounds.y, bounds.w, bounds.h) });
    }
}

lunasvg::Matrix RetroFuturaGUI::SvgTexture::computeScaleMatrix(glm::i32vec2 targetResolution) const
{
    return lunasvg::Matrix::scaled(
        static_cast<f32>(targetResolution.x) / _documentIntrinsicSize.x,
        static_cast<f32>(targetResolution.y) / _documentIntrinsicSize.y);
}

void RetroFuturaGUI::SvgTexture::rasterizeBaseLayer(glm::i32vec2 targetResolution)
{
    if (!_document || targetResolution.x <= 0 || targetResolution.y <= 0)
        return;

    lunasvg::Bitmap bitmap(targetResolution.x, targetResolution.y);
    if (bitmap.isNull())
    {
        std::println("Error rasterizing SVG base layer: {}", _path);
        return;
    }

    bitmap.clear(0x00000000);
    _document->render(bitmap, computeScaleMatrix(targetResolution));
    bitmap.convertToRGBA();

    const uSize rowBytes = static_cast<uSize>(targetResolution.x) * 4;
    const uSize srcStride = static_cast<uSize>(bitmap.stride());
    std::vector<u8> pixelData(rowBytes * static_cast<uSize>(targetResolution.y));

    for (i32 row = 0; row < targetResolution.y; ++row)
    {
        const u8* srcRow = bitmap.data() + static_cast<uSize>(row) * srcStride;
        u8* dstRow = pixelData.data() +
            static_cast<uSize>(_verticallyFlipped ? (targetResolution.y - 1 - row) : row) * rowBytes;
        memcpy_s(dstRow, rowBytes, srcRow, rowBytes);
    }

    uploadBaseLayer(pixelData, targetResolution);
}

void RetroFuturaGUI::SvgTexture::materializeMask(PathLayer& layer, glm::i32vec2 targetResolution)
{
    if (!_document || targetResolution.x <= 0 || targetResolution.y <= 0)
        return;

    lunasvg::Bitmap bitmap(targetResolution.x, targetResolution.y);
    if (bitmap.isNull())
        return;

    // Element::render() renders in the element's own local coordinate space and does not
    // resolve the document's viewBox-to-width/height scale the way Document::render() does,
    // so isolating the element and rendering it through the (already-correct) document path
    // is what keeps a mask aligned with the base layer for any SVG where viewBox != width/height.
    std::vector<lunasvg::Element> ancestorChain;
    for (lunasvg::Element current = layer.element; !current.isNull(); current = current.parentElement())
        ancestorChain.push_back(current);

    std::vector<std::pair<lunasvg::Element, std::string>> hiddenSiblings;
    for (uSize i = 0; i + 1 < ancestorChain.size(); ++i)
    {
        for (const lunasvg::Node& child : ancestorChain[i + 1].children())
        {
            if (!child.isElement())
                continue;

            lunasvg::Element childElement = child.toElement();
            if (childElement == ancestorChain[i])
                continue;

            hiddenSiblings.push_back({ childElement,
                childElement.hasAttribute("display") ? childElement.getAttribute("display") : std::string() });
            childElement.setAttribute("display", "none");
        }
    }

    bitmap.clear(0x00000000);
    _document->render(bitmap, computeScaleMatrix(targetResolution));

    for (auto& [element, originalDisplay] : hiddenSiblings)
        element.setAttribute("display", originalDisplay.empty() ? "inline" : originalDisplay);

    const uSize srcStride = static_cast<uSize>(bitmap.stride());
    const uSize width = static_cast<uSize>(targetResolution.x);
    std::vector<u8> alphaOnly(width * static_cast<uSize>(targetResolution.y));

    for (i32 row = 0; row < targetResolution.y; ++row)
    {
        const u8* srcRow = bitmap.data() + static_cast<uSize>(row) * srcStride;
        u8* dstRow = alphaOnly.data() +
            static_cast<uSize>(_verticallyFlipped ? (targetResolution.y - 1 - row) : row) * width;

        for (i32 col = 0; col < targetResolution.x; ++col)
            dstRow[col] = srcRow[col * 4 + 3];
    }

    uploadMask(layer, alphaOnly, targetResolution);
}

void RetroFuturaGUI::SvgTexture::uploadBaseLayer(const std::vector<u8>& rgba, glm::i32vec2 resolution)
{
    if (_baseLayerTextureId == 0)
        glGenTextures(1, &_baseLayerTextureId);

    glBindTexture(GL_TEXTURE_2D, _baseLayerTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    _resolution = resolution;
    _aspectRatio = static_cast<f32>(resolution.x) / static_cast<f32>(resolution.y);
}

void RetroFuturaGUI::SvgTexture::uploadMask(PathLayer& layer, const std::vector<u8>& alphaOnly, glm::i32vec2 resolution)
{
    if (layer.maskTextureId == 0)
        glGenTextures(1, &layer.maskTextureId);

    glBindTexture(GL_TEXTURE_2D, layer.maskTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // R8 rows of arbitrary width aren't 4-byte aligned; every other texture upload in this
    // codebase is RGBA, so the default alignment must be restored afterward.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, resolution.x, resolution.y, 0, GL_RED, GL_UNSIGNED_BYTE, alphaOnly.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void RetroFuturaGUI::SvgTexture::setupQuad()
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices), _indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

RetroFuturaGUI::SvgTexture::PathLayer* RetroFuturaGUI::SvgTexture::findLayer(std::string_view name)
{
    for (PathLayer& layer : _layers)
        if (layer.name == name)
            return &layer;

    return nullptr;
}
