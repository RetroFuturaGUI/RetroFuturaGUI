#pragma once
#include "ITexture.hpp"
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    enum class ImageFormat : i32
    {
        Unknown = -1,
        PNG = 0,
        JPG,
        JPEG = JPG,
        SVG
    };

    /// @brief An image decoded into memory without GL, so a worker thread can decode it and the thread holding the GL context
    ///        upload it: RGBA, 4 bytes a pixel, with straight alpha.
    struct DecodedImage
    {
        std::vector<u8> _Pixels {};
        glm::i32vec2
            _Resolution { 0 },
            _SourceResolution { 0 }; //the image's own size, before it was scaled to fit; an SVG's is its document's
        bool _IsFlipped { false }; //the bottom row comes first, the way OpenGL samples a texture
    };

    /// @brief A texture loaded once from an image file and uploaded to the GPU: every format stb_image reads (PNG, JPG, BMP,
    ///        GIF's first frame, TGA, PSD, HDR, PIC, PNM) and SVG.
    class Texture final : public ITexture
    {
    public:
        /// @brief Loads a texture from disk and uploads it to the GPU.
        /// @param path UTF-8.
        /// @param projection Only needed if this texture will draw itself as a standalone quad (e.g. Image).
        /// Textures used purely as material maps (e.g. Mesh) can omit it.
        Texture(std::string_view path, const bool flipVertically = true, Projection* projection = nullptr);

        /// @brief Uploads an image decoded before, on whichever thread decoded it. Needs a GL context current.
        explicit Texture(DecodedImage&& image, Projection* projection = nullptr);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        ~Texture() override = default; // the GL objects are ITexture's

        /// @brief Decodes an image file into RGBA pixels without touching GL, so it may run on any thread. An SVG is told by
        ///        its extension. Never throws.
        /// @param maxSide When above 0, a raster image bigger than it is scaled down to fit inside a square that size, and an
        ///        SVG is drawn to fit it; otherwise both keep their own size.
        /// @return Nothing when the file can't be read or decoded.
        static std::optional<DecodedImage> DecodeFile(const std::filesystem::path& path, const bool flipVertically, const i32 maxSide = 0);

        /// @brief Decodes an image held in memory, the cover art embedded in a song for instance, as DecodeFile does a file. An
        ///        SVG is told by its text.
        static std::optional<DecodedImage> DecodeMemory(std::span<const u8> bytes, const bool flipVertically, const i32 maxSide = 0);

        /// @brief Returns the number of color channels in the texture, 4: every image is uploaded as RGBA.
        i32 GetColorChannelCount() const;

        /// @brief Returns the raw pixel data - empty once uploaded, the GPU keeps the only copy.
        std::vector<u8>* GetTextureData();

        /// @brief Returns the filesystem path the texture was loaded from.
        std::string_view GetPath() const;

        /// @brief Sets a free-form type tag for the texture (e.g. "diffuse", "specular").
        void SetType(std::string_view type);

        /// @brief Returns the type tag of the texture.
        std::string_view GetType() const;

    private:
        /// @brief Takes over a decoded image's pixels and size, ready to upload.
        void takeImage(DecodedImage&& image);
        void uploadToGPU();

        static std::optional<DecodedImage> decodeRaster(std::span<const u8> bytes, const bool flipVertically, const i32 maxSide);
        static std::optional<DecodedImage> decodeSvg(std::span<const u8> bytes, const bool flipVertically, const i32 maxSide);

        /// @brief The path's extension in lowercase ASCII, ".png".
        static std::string lowerExtension(const std::filesystem::path& path);

        /// @brief Returns whether the bytes start like an SVG document, after any byte order mark and white space.
        static bool looksLikeSvg(std::span<const u8> bytes);

        /// @brief Scales RGBA pixels down so neither side is longer than maxSide, each pixel the average of the ones it covers,
        ///        weighted by their alpha so transparent pixels don't darken the edges.
        static void shrinkToFit(DecodedImage& image, const i32 maxSide);

        /// @brief Puts the bottom row first.
        static void flipRows(DecodedImage& image);

        ImageFormat _format { ImageFormat::Unknown };
        i32 _colorChannelCount { 0 };
        std::vector<u8> _texture {};
        std::string
            _path {},
            _type {};
    };
}