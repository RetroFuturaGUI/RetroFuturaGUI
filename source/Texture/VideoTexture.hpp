#pragma once
#include "ITexture.hpp"
#include "config.hpp"
#include <array>

// FFmpeg's types, declared rather than included: only VideoTexture.cpp needs their insides
struct AVFrame;
struct SwsContext;

namespace RetroFuturaGUI
{
    /// @brief A texture whose pictures come from a video.
    class VideoTexture final : public ITexture
    {
    public:
        /// @param projection Only needed if this texture will draw itself as a standalone quad (e.g. Image).
        /// Textures used purely as material maps (e.g. Mesh) can omit it.
        explicit VideoTexture(Projection* projection = nullptr);
        VideoTexture(const VideoTexture&) = delete;
        VideoTexture& operator=(const VideoTexture&) = delete;
        VideoTexture(VideoTexture&&) = delete;
        VideoTexture& operator=(VideoTexture&&) = delete;
        ~VideoTexture() override;

        /// @brief Shows a decoded picture: uploads its planes and converts them into the texture.
        bool Update(const AVFrame* frame);

        /// @brief Whether a picture has been shown yet. Before the first Update the texture is empty.
        bool HasPicture() const;

    private:
        /// @brief (Re)creates the RGBA texture, the plane textures and the framebuffer for a picture size.
        bool allocate(const i32 width, const i32 height);
        const AVFrame* toYuv420p(const AVFrame* frame);
        void uploadPlanes(const AVFrame* frame);
        void convert(const AVFrame* frame);
        void releaseVideoResources();

        std::array<u32, 3> _planeTextureIds {};
        std::unique_ptr<Framebuffer> _framebuffer { nullptr }; //one per context that converts a picture, all rendering into the same texture
        std::unique_ptr<VertexArray> _emptyVertexArray { nullptr }; //the conversion pass makes its triangle in the vertex shader, so there is no layout
        SwsContext* _converter { nullptr };
        AVFrame* _convertedFrame { nullptr };
        bool _hasPicture { false };
    };
}