#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.hpp"
#include <print>
#include <lunasvg.h>

RetroFuturaGUI::Texture::Texture(std::string_view path, const bool flipVertically, Projection* projection)
    : ITexture(projection, flipVertically)
{
    loadTexture(path);
    _aspectRatio = static_cast<f32>(_resolution.x) / static_cast<f32>(_resolution.y);
    uploadToGPU();
    setupQuad(); // only builds the quad with a Projection
}

RetroFuturaGUI::Texture::Texture(Texture&& other) noexcept
    : ITexture(std::move(other)), _format(other._format), _colorChannelCount(other._colorChannelCount),
      _texture(std::move(other._texture)), _path(std::move(other._path)), _type(std::move(other._type))
{}

RetroFuturaGUI::Texture& RetroFuturaGUI::Texture::operator=(Texture&& other) noexcept
{
    if(this == &other)
        return *this;

    ITexture::operator=(std::move(other)); // hands over the GL objects, releasing this one's first
    _format = other._format;
    _colorChannelCount = other._colorChannelCount;
    _texture = std::move(other._texture);
    _path = std::move(other._path);
    _type = std::move(other._type);
    return *this;
}

i32 RetroFuturaGUI::Texture::GetColorChannelCount() const
{
    return _colorChannelCount;
}

std::vector<u8>* RetroFuturaGUI::Texture::GetTextureData()
{
    return &_texture;
}

void RetroFuturaGUI::Texture::uploadToGPU()
{
    if (_texture.empty())
        return;

    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    u32 format = (_colorChannelCount == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<i32>(format), _resolution.x, _resolution.y, 0, format, GL_UNSIGNED_BYTE, _texture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // The GPU now owns the pixel data; drop the CPU-side copy instead of holding both forever
    _texture.clear();
    _texture.shrink_to_fit();
}

void RetroFuturaGUI::Texture::loadTexture(std::string_view path)
{
    _path = path;
    std::string extension { path.substr(path.find_last_of('.') + 1) };
    std::transform(extension.begin(), extension.end(), extension.begin(), [](uChar c) { return std::tolower(c); });

    if (extension == "svg")
        _format = ImageFormat::SVG;
    else if (extension == "png")
        _format = ImageFormat::PNG;
    else if (extension == "jpg" || extension == "jpeg")
        _format = ImageFormat::JPG;
    else
        _format = ImageFormat::Unknown;

    switch(_format)
    {
        case ImageFormat::SVG:
            loadSVG(_path);
            break;
        case ImageFormat::PNG:
        case ImageFormat::JPG:
            loadRasterImage(_path);
            break;
        default:
            std::println("Unsupported image format: {}", extension);
            break;
    }
}

void RetroFuturaGUI::Texture::loadRasterImage(std::string_view path)
{
    stbi_set_flip_vertically_on_load(_verticallyFlipped);
    u8* tempData = stbi_load(path.data(), &_resolution.x, &_resolution.y, &_colorChannelCount, 0);

    if (tempData)
        _texture = std::vector<u8>(tempData, tempData + _resolution.x * _resolution.y * _colorChannelCount);
    else
        std::println("Error loading image: {}", path);

    if(tempData)
        stbi_image_free(tempData);
}

void RetroFuturaGUI::Texture::loadSVG(std::string_view path)
{
    std::unique_ptr<lunasvg::Document> document = lunasvg::Document::loadFromFile(path.data());
    if (!document)
    {
        std::println("Error loading SVG: {}", path);
        return;
    }

    _resolution.x = static_cast<i32>(document->width());
    _resolution.y = static_cast<i32>(document->height());
    _colorChannelCount = 4;
    lunasvg::Bitmap bitmap { document->renderToBitmap() };

    if(bitmap.isNull())
        return;

    const u32 width = static_cast<u32>(bitmap.width());
    const u32 height = static_cast<u32>(bitmap.height());

    /* The pixels below are laid out to the bitmap's dimensions, so the resolution has to come from the
       bitmap too. Taking it from the document truncates its float size - a 255.99998 wide document
       reports 255 against a 256 wide bitmap - and the upload then reads every row one pixel short,
       shearing the image diagonally. */
    _resolution.x = static_cast<i32>(width);
    _resolution.y = static_cast<i32>(height);
    const uSize srcStride = static_cast<uSize>(bitmap.stride());
    const uSize rowBytes = static_cast<uSize>(width) * _colorChannelCount;
    std::vector<u8> pixelData(rowBytes * height);

    for (u32 row = 0; row < height; ++row)
    {
        const u8* srcRow = bitmap.data() + row * srcStride;
        u8* dstRow = pixelData.data() + (_verticallyFlipped ? (height - 1 - row) : row) * rowBytes;

        for (u32 col = 0; col < width; ++col)
        {
            const u8* srcPixel = srcRow + col * _colorChannelCount;
            u8* dstPixel = dstRow + col * _colorChannelCount;

            const u8 b = srcPixel[0];
            const u8 g = srcPixel[1];
            const u8 r = srcPixel[2];
            const u8 a = srcPixel[3];

            if (a == 0)
                dstPixel[0] = dstPixel[1] = dstPixel[2] = dstPixel[3] = 0;
            else
            {
                dstPixel[0] = static_cast<u8>(std::min(255, r * 255 / a));
                dstPixel[1] = static_cast<u8>(std::min(255, g * 255 / a));
                dstPixel[2] = static_cast<u8>(std::min(255, b * 255 / a));
                dstPixel[3] = a;
            }
        }
    }

    _texture = std::move(pixelData);
}

std::string_view RetroFuturaGUI::Texture::GetPath() const
{
    return _path;
}

void RetroFuturaGUI::Texture::SetType(std::string_view type)
{
    _type = type;
}

std::string_view RetroFuturaGUI::Texture::GetType() const
{
    return _type;
}