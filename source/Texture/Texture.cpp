#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.hpp"
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <system_error>
#include <print>
#include <lunasvg.h>

RetroFuturaGUI::Texture::Texture(std::string_view path, const bool flipVertically, Projection* projection)
    : ITexture(projection, flipVertically)
{
    _path = path;

    //UTF-8 into a native path, so names outside the system's code page open too
    const std::filesystem::path filePath { std::u8string(path.begin(), path.end()) };
    const std::string extension { lowerExtension(filePath) };

    if(".svg" == extension)
        _format = ImageFormat::SVG;
    else if(".png" == extension)
        _format = ImageFormat::PNG;
    else if(".jpg" == extension || ".jpeg" == extension)
        _format = ImageFormat::JPG;

    std::optional<DecodedImage> image { DecodeFile(filePath, flipVertically) };

    if(image)
        takeImage(std::move(*image));
    else
        std::println("Error loading image: {}", path);

    uploadToGPU();
    setupQuad(); // only builds the quad with a Projection
}

RetroFuturaGUI::Texture::Texture(DecodedImage&& image, Projection* projection)
    : ITexture(projection, image._IsFlipped)
{
    takeImage(std::move(image));
    uploadToGPU();
    setupQuad();
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

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::Texture::DecodeFile(const std::filesystem::path& path, const bool flipVertically, const i32 maxSide)
{
    //Read whole, so a file named like an image but gigabytes big would fill the memory first
    std::error_code error {};
    const std::uintmax_t size { std::filesystem::file_size(path, error) };

    if(error || _maxFileBytes < size)
        return std::nullopt;

    std::ifstream file { path, std::ios::binary };

    if(!file)
        return std::nullopt;

    const std::vector<u8> bytes { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

    if(".svg" == lowerExtension(path))
        return decodeSvg(bytes, flipVertically, maxSide);

    return decodeRaster(bytes, flipVertically, maxSide);
}

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::Texture::DecodeMemory(std::span<const u8> bytes, const bool flipVertically, const i32 maxSide)
{
    if(looksLikeSvg(bytes))
        return decodeSvg(bytes, flipVertically, maxSide);

    return decodeRaster(bytes, flipVertically, maxSide);
}

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::Texture::decodeRaster(std::span<const u8> bytes, const bool flipVertically, const i32 maxSide)
{
    if(bytes.empty())
        return std::nullopt;

    if(static_cast<uSize>((std::numeric_limits<i32>::max)()) < bytes.size())
        return std::nullopt; //stb_image takes the length as an int

    i32
        width { 0 },
        height { 0 },
        channelCount { 0 };

    //The size the image declares, read before anything is allocated for it: a few bytes can claim a picture of gigabytes
    if(!stbi_info_from_memory(bytes.data(), static_cast<i32>(bytes.size()), &width, &height, &channelCount))
        return std::nullopt;

    if(!isDecodableSize(width, height))
        return std::nullopt;

    //Always four channels, so a grayscale or gray and alpha image uploads like any other, and every row is 4-byte aligned
    u8* decoded { stbi_load_from_memory(bytes.data(), static_cast<i32>(bytes.size()), &width, &height, &channelCount, 4) };

    if(!decoded)
        return std::nullopt;

    DecodedImage image { ._Pixels = std::vector<u8>(decoded, decoded + static_cast<uSize>(width) * static_cast<uSize>(height) * 4), ._Resolution = glm::i32vec2(width, height), ._SourceResolution = glm::i32vec2(width, height), ._IsFlipped = false };
    stbi_image_free(decoded);
    shrinkToFit(image, maxSide);

    if(flipVertically)
        flipRows(image);

    return image;
}

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::Texture::decodeSvg(std::span<const u8> bytes, const bool flipVertically, const i32 maxSide)
{
    std::unique_ptr<lunasvg::Document> document { lunasvg::Document::loadFromData(reinterpret_cast<const char*>(bytes.data()), bytes.size()) };

    if(!document)
        return std::nullopt;

    //Drawn to fit the side asked for, which keeps it sharp at any size; otherwise at the size the document gives
    const f32
        documentWidth { document->width() },
        documentHeight { document->height() },
        longest { std::max(documentWidth, documentHeight) },
        scale { 0 < maxSide && 0.0f < longest ? static_cast<f32>(maxSide) / longest : 1.0f };
    const f64
        drawnWidth { static_cast<f64>(documentWidth) * scale },
        drawnHeight { static_cast<f64>(documentHeight) * scale };

    //A document can declare any size, so it's checked in floating point before rounding to pixels; NaN fails it too
    if(!(static_cast<f64>(_maxDecodedPixels) >= drawnWidth * drawnHeight))
        return std::nullopt;

    const i64
        renderWidth { std::llround(drawnWidth) },
        renderHeight { std::llround(drawnHeight) };

    if(!isDecodableSize(renderWidth, renderHeight))
        return std::nullopt;

    const lunasvg::Bitmap bitmap { document->renderToBitmap(static_cast<i32>(renderWidth), static_cast<i32>(renderHeight)) };

    if(bitmap.isNull())
        return std::nullopt;

    /* The pixels below are laid out to the bitmap's dimensions, so the resolution has to come from the
       bitmap too. Taking it from the document truncates its float size - a 255.99998 wide document
       reports 255 against a 256 wide bitmap - and the upload then reads every row one pixel short,
       shearing the image diagonally. */
    const u32
        width { static_cast<u32>(bitmap.width()) },
        height { static_cast<u32>(bitmap.height()) };
    const uSize
        sourceStride { static_cast<uSize>(bitmap.stride()) },
        rowBytes { static_cast<uSize>(width) * 4 };
    DecodedImage image { ._Pixels = std::vector<u8>(rowBytes * height), ._Resolution = glm::i32vec2(static_cast<i32>(width), static_cast<i32>(height)), ._SourceResolution = glm::i32vec2(static_cast<i32>(std::lround(documentWidth)), static_cast<i32>(std::lround(documentHeight))), ._IsFlipped = false };

    //lunasvg writes premultiplied BGRA; the texture wants straight RGBA
    for(u32 row = 0; row < height; ++row)
    {
        const u8* sourceRow { bitmap.data() + row * sourceStride };
        u8* targetRow { image._Pixels.data() + row * rowBytes };

        for(u32 column = 0; column < width; ++column)
        {
            const u8* source { sourceRow + column * 4 };
            u8* target { targetRow + column * 4 };
            const u8 alpha { source[3] };

            if(0 == alpha)
            {
                target[0] = target[1] = target[2] = target[3] = 0;
                continue;
            }

            target[0] = static_cast<u8>(std::min(255, source[2] * 255 / alpha));
            target[1] = static_cast<u8>(std::min(255, source[1] * 255 / alpha));
            target[2] = static_cast<u8>(std::min(255, source[0] * 255 / alpha));
            target[3] = alpha;
        }
    }

    if(flipVertically)
        flipRows(image);

    return image;
}

std::string RetroFuturaGUI::Texture::lowerExtension(const std::filesystem::path& path)
{
    //Through UTF-8, since a native string in the system's code page throws on characters it doesn't have
    const std::u8string text { path.extension().u8string() };
    std::string extension(text.begin(), text.end());
    std::transform(extension.begin(), extension.end(), extension.begin(), [](const char character) { return 'A' <= character && 'Z' >= character ? static_cast<char>(character - 'A' + 'a') : character; });
    return extension;
}

bool RetroFuturaGUI::Texture::looksLikeSvg(std::span<const u8> bytes)
{
    uSize start { 0 };

    if(3 <= bytes.size() && 0xEF == bytes[0] && 0xBB == bytes[1] && 0xBF == bytes[2])
        start = 3; //UTF-8's byte order mark

    while(start < bytes.size() && std::isspace(bytes[start]))
        ++start;

    const std::string_view text { reinterpret_cast<const char*>(bytes.data()) + start, bytes.size() - start };
    return text.starts_with("<svg") || (text.starts_with("<?xml") && std::string_view::npos != text.substr(0, 1024).find("<svg"));
}

void RetroFuturaGUI::Texture::shrinkToFit(DecodedImage& image, const i32 maxSide)
{
    const i32
        width { image._Resolution.x },
        height { image._Resolution.y },
        longest { std::max(width, height) };

    if(0 >= maxSide || longest <= maxSide)
        return;

    const f64 scale { static_cast<f64>(maxSide) / static_cast<f64>(longest) };
    const i32
        targetWidth { std::max(1, static_cast<i32>(std::lround(width * scale))) },
        targetHeight { std::max(1, static_cast<i32>(std::lround(height * scale))) };
    std::vector<u8> pixels(static_cast<uSize>(targetWidth) * static_cast<uSize>(targetHeight) * 4);

    //Each target pixel averages the source rectangle it covers
    for(i32 targetY = 0; targetY < targetHeight; ++targetY)
    {
        const i32
            sourceTop { targetY * height / targetHeight },
            sourceBottom { std::max(sourceTop + 1, (targetY + 1) * height / targetHeight) };

        for(i32 targetX = 0; targetX < targetWidth; ++targetX)
        {
            const i32
                sourceLeft { targetX * width / targetWidth },
                sourceRight { std::max(sourceLeft + 1, (targetX + 1) * width / targetWidth) };
            u64
                red { 0 },
                green { 0 },
                blue { 0 },
                alpha { 0 },
                pixelCount { 0 };

            for(i32 sourceY = sourceTop; sourceY < sourceBottom; ++sourceY)
            {
                const u8* sourceRow { image._Pixels.data() + static_cast<uSize>(sourceY) * static_cast<uSize>(width) * 4 };

                for(i32 sourceX = sourceLeft; sourceX < sourceRight; ++sourceX)
                {
                    const u8* source { sourceRow + static_cast<uSize>(sourceX) * 4 };
                    red += static_cast<u64>(source[0]) * source[3];
                    green += static_cast<u64>(source[1]) * source[3];
                    blue += static_cast<u64>(source[2]) * source[3];
                    alpha += source[3];
                    ++pixelCount;
                }
            }

            u8* target { pixels.data() + (static_cast<uSize>(targetY) * static_cast<uSize>(targetWidth) + static_cast<uSize>(targetX)) * 4 };
            target[0] = static_cast<u8>(0 < alpha ? red / alpha : 0);
            target[1] = static_cast<u8>(0 < alpha ? green / alpha : 0);
            target[2] = static_cast<u8>(0 < alpha ? blue / alpha : 0);
            target[3] = static_cast<u8>(alpha / pixelCount);
        }
    }

    image._Pixels = std::move(pixels);
    image._Resolution = glm::i32vec2(targetWidth, targetHeight);
}

bool RetroFuturaGUI::Texture::isDecodableSize(const i64 width, const i64 height)
{
    return 0 < width && 0 < height && _maxDecodedPixels / width >= height; //divided, so a huge side can't overflow the product
}

void RetroFuturaGUI::Texture::flipRows(DecodedImage& image)
{
    const uSize rowBytes { static_cast<uSize>(image._Resolution.x) * 4 };

    for(i32 top = 0, bottom = image._Resolution.y - 1; top < bottom; ++top, --bottom)
        std::swap_ranges(image._Pixels.begin() + static_cast<std::ptrdiff_t>(top * rowBytes), image._Pixels.begin() + static_cast<std::ptrdiff_t>((top + 1) * rowBytes), image._Pixels.begin() + static_cast<std::ptrdiff_t>(bottom * rowBytes));

    image._IsFlipped = !image._IsFlipped;
}

void RetroFuturaGUI::Texture::takeImage(DecodedImage&& image)
{
    _resolution = image._Resolution;
    _aspectRatio = 0 < _resolution.y ? static_cast<f32>(_resolution.x) / static_cast<f32>(_resolution.y) : 0.0f;
    _colorChannelCount = 4;
    _texture = std::move(image._Pixels);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _resolution.x, _resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, _texture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // The GPU now owns the pixel data; drop the CPU-side copy instead of holding both forever
    _texture.clear();
    _texture.shrink_to_fit();
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